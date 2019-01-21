
class Surround : public IXAudio2EngineCallback {
    Microsoft::WRL::ComPtr<IXAudio2> xaudio2;
    IXAudio2MasteringVoice* master_voice = nullptr;

    void EnableDebug() {
        XAUDIO2_DEBUG_CONFIGURATION config;
        config.TraceMask = XAUDIO2_LOG_WARNINGS | XAUDIO2_LOG_DETAIL;
        config.BreakMask = XAUDIO2_LOG_WARNINGS;
        config.LogThreadID = TRUE;
        config.LogFileline = TRUE;
        config.LogFunctionName = TRUE;
        config.LogTiming = TRUE;
        xaudio2->SetDebugConfiguration(&config, NULL);
    }

public:
    static void CheckResult(HRESULT hresult) {
        DEBUG_ONLY(if (FAILED(hresult)) throw Exception();)
    }

    Surround() {
        CheckResult(CoInitialize(nullptr));
        CheckResult(XAudio2Create(&xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR));
        DEBUG_ONLY(EnableDebug());
        CheckResult(xaudio2->RegisterForCallbacks(this));
        CheckResult(xaudio2->CreateMasteringVoice(&master_voice));
    }

    ~Surround() {
        xaudio2.Reset();
        CoUninitialize();
    }

    void Start() {
        xaudio2->StartEngine();
    }

    void Stop() {
        xaudio2->StopEngine();
    }

    void OnProcessingPassStart() final {

    }

    void OnProcessingPassEnd() final {

    }

    void OnCriticalError(HRESULT Error) final {

    }

    Microsoft::WRL::ComPtr<IXAudio2> XAudio2() { return xaudio2; }
};

class BankDynamic : public Bank {
    Bundle::Asset asset;

public:
    BankDynamic(uint64 id, const String& name)
        : Bank(id, name) {}

    void Load(const Bundle& bundle, Surround& surround) {
        asset = bundle.FindData(Id());
    }

    const uint8* Buffer(const Sound& sound) const { return asset.Mem() + sound.Offset(); }
};

class Voice : public IXAudio2VoiceCallback {
    IXAudio2SourceVoice* source_voice = nullptr;
    bool end = false;

    bool Start(Surround& surround, const Source& source, const BankDynamic& bank, uint32 sound_id, float volume) {
        if (auto* sound = bank.FindSound(sound_id)) {
            Surround::CheckResult(surround.XAudio2()->CreateSourceVoice(&source_voice, (WAVEFORMATEX*)sound->RawFormat(), 0, XAUDIO2_DEFAULT_FREQ_RATIO, this));

            XAUDIO2_BUFFER buffer = { 0 };
            buffer.pAudioData = (BYTE*)bank.Buffer(*sound);
            buffer.Flags = XAUDIO2_END_OF_STREAM;
            buffer.AudioBytes = (UINT)sound->Length();

            Surround::CheckResult(source_voice->SubmitSourceBuffer(&buffer));
            Surround::CheckResult(source_voice->SetVolume(volume));
            Surround::CheckResult(source_voice->Start(0));
            end = false;
            return true;
        }
        return false;
    }

    void Stop() {
        if (source_voice) {
            Surround::CheckResult(source_voice->Stop());
            Surround::CheckResult(source_voice->FlushSourceBuffers());
            source_voice->DestroyVoice();
            source_voice = nullptr;
        }
    }

    void OnVoiceProcessingPassStart(UINT32 BytesRequired) final {
    }

    void OnVoiceProcessingPassEnd() final {
    }

    void OnStreamEnd() final {
        end = true;
    }

    void OnBufferStart(void* pBufferContext) final {
    }

    void OnBufferEnd(void* pBufferContext) final {
    }

    void OnLoopEnd(void* pBufferContext) final {
    }

    void OnVoiceError(void* pBufferContext, HRESULT Error) final {
    }

public:
    ~Voice() {
        Stop();
    }

    bool Play(Surround& surround, const Source& source, const BankDynamic& bank, uint32 sound_id, float volume) {
        Stop();
        return Start(surround, source, bank, sound_id, volume);
    }

    void GarbageCollect() {
        if (end)
            Stop();
    }
};
