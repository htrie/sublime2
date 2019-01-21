
class ScriptDynamic : public Script {
    Library library;
    InitFunc init_func = nullptr;
    MouseFunc mouse_func = nullptr;
    KeyboardFunc keyboard_func = nullptr;
    UpdateFunc update_func = nullptr;

    String Create(const Bundle& bundle) {
        auto normalized_name = Name();
        normalized_name.Replace('/', '_');
        const auto dll_filename = normalized_name + ".dll";
        const auto asset = bundle.FindData(Id());
        WriteOnlyFile dll_file(dll_filename, asset.Size());
        memcpy(dll_file.Pointer(), asset.Mem(), asset.Size());
        return dll_filename;
    }

public:
    ScriptDynamic(uint64 id, const String& name)
        : Script(id, name) {}

    void Load(const Bundle& bundle) {
        const auto data_filename = Create(bundle);
        new (&library) Library(data_filename.Data());
        init_func = library.Address<InitFunc>("Init");
        mouse_func = library.Address<MouseFunc>("Mouse");
        keyboard_func = library.Address<KeyboardFunc>("Keyboard");
        update_func = library.Address<UpdateFunc>("Update");
    }

    void Init(Commands& commands) {
        if (init_func)
            init_func(commands);
    }

    void Mouse(MousePhase phase, Button button, float u, float v, float w) const {
        if (mouse_func)
            mouse_func(phase, button, u, v, w);
    }

    void Keyboard(KeyboardPhase phase, Key key) const {
        if (keyboard_func)
            keyboard_func(phase, key);
    }

    void Execute(float elapsed_time, unsigned window_witdh, unsigned window_height) const {
        if (update_func)
            update_func(elapsed_time, window_witdh, window_height);
    }
};
