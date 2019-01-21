
extern void Game_Init(Commands& _commands);
extern void Game_Mouse(MousePhase phase, Button button, float u, float v, float w);
extern void Game_Keyboard(KeyboardPhase phase, Key key);
extern void Game_Update(float timestep, unsigned window_witdh, unsigned window_height);

class ScriptDynamic : public Script {
    enum Type {
        Unknown = 0,
        Game,
    };
    Type type = Unknown; // TODO: Remove.
    
public:
    void Load(Bundle& bundle, Commands& commands) {
        if (Name() == "Scripts/Game.script") { type = Game; }
        switch (type) {
            case Game: Game_Init(commands); break;
            default: break;
        }
    }
    
    void Mouse(MousePhase phase, Button button, float u, float v, float w) {
        if (Name() == "Game.script") { type = Game; }
        switch (type) {
            case Game: Game_Mouse(phase, button, u, v, w); break;
            default: break;
        }
    }
    
    void Keyboard(KeyboardPhase phase, Key key) {
        if (Name() == "Game.script") { type = Game; }
        switch (type) {
            case Game: Game_Keyboard(phase, key); break;
            default: break;
        }
    }
    
    void Execute(float elapsed_time, unsigned window_witdh, unsigned window_height) {
        if (Name() == "Game.script") { type = Game; }
        switch (type) {
            case Game: Game_Update(elapsed_time, window_witdh, window_height); break;
            default: break;
        }
    }
};
