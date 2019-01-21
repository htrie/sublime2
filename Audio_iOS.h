
class Surround {
public:
    Surround() {
    }
    
    ~Surround() {
    }
    
    void Start() {
    }
    
    void Stop() {
    }
};

class BankDynamic : public Bank {
public:
    void Load(Bundle& bundle, Surround& surround) {
    }
};

class Voice {
public:
    ~Voice() {
    }
    
    bool Play(Surround& surround, const Source& source, const Bank& bank, uint32 sound_id, float volume) {
        return false;
    }
    
    void GarbageCollect() {
    }
};
