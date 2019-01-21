
static const unsigned version_major = 5;
static const unsigned version_minor = 0;
static const unsigned version_patch = 0;

enum class MousePhase : uint8 {
    None = 0,
    Down,
    Dragged,
    Up,
    Scroll,
};

enum class Button : uint8 {
    None = 0,
    Left,
    Middle,
    Right,
    Wheel,
};

enum class KeyboardPhase : uint8 {
    None = 0,
    Pressed,
    Released,
    Char,
};

enum class Key : uint8 {
    None = 0,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    CONTROL,
    BACKSPACE,
    LEFT,
    UP,
    RIGHT,
    DOWN,
    PAGE_UP,
    PAGE_DOWN,
    HOME,
    END,
    INSERT,
    SHIFT,
    SPACE,
    COMMA,
    POINT,
    LESS,
    GREATER,
    SLASH,
    BACKSLASH,
    INTERROGATION,
    PLUS,
    MINUS,
    UNDERSCORE,
    EQUALS,
    a,
    b,
    c,
    d,
    e,
    f,
    g,
    h,
    i,
    j,
    k,
    l,
    m,
    n,
    o,
    p,
    q,
    r,
    s,
    t,
    u,
    v,
    w,
    x,
    y,
    z,
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    N0,
    N1,
    N2,
    N3,
    N4,
    N5,
    N6,
    N7,
    N8,
    N9,
};

struct Id {
    uint64 cluster_id = 0;
    uint64 batch_id = 0;
    uint32 instance_id = 0;

    Id() {}
    Id(uint64 cluster_id, uint64 batch_id, uint32 instance_id)
        : cluster_id(cluster_id), batch_id(batch_id), instance_id(instance_id) {}
    Id(const char* cluster_name, const char* batch_name, uint32 instance_id)
        : cluster_id(Hash::Fnv64(cluster_name)), batch_id(Hash::Fnv64(batch_name)), instance_id(instance_id) {}

    void Clear() {
        cluster_id = 0;
        batch_id = 0;
        instance_id = 0;
    }

    operator bool() const { return (cluster_id != 0) || (batch_id != 0); }

    bool operator!=(const Id& other) const { return (cluster_id != other.cluster_id) || (batch_id != other.batch_id) || (instance_id != other.instance_id); }
    bool operator==(const Id& other) const { return (cluster_id == other.cluster_id) && (batch_id == other.batch_id) && (instance_id == other.instance_id); }
};

typedef bool(*IsRelease)();
typedef void(*ToggleProfileJobs)();
typedef void(*ToggleDrawBounds)();
typedef void(*ToggleDrawExtents)();
typedef uint64(*DataId)(const char* name);
typedef float(*GetValue)(uint64 dictionary_id, uint32 hash);
typedef Vector2(*GetDynamicScale)();
typedef float(*GetAspectRatio)();
typedef Vector2(*GetWindowSize)();
typedef uint64(*GetGpuFrameDuration)();
typedef void(*SetDynamicScale)(float scale);
typedef Quaternion(*GetRotation)(const Id& id);
typedef Vector3(*GetPosition)(const Id& id);
typedef void(*SetPosition)(const Id& id, const Vector3& position);
typedef void(*SetRotation)(const Id& id, const Quaternion& rotation);
typedef Id(*Pick)(const Id& camera_id, float u, float v, unsigned flags, Ray& out_ray);
typedef void(*SwapSurface)(const Id& id, unsigned index, uint64 texture_id);
typedef void(*SwapUniforms)(const Id& id, uint64 uniforms_id);
typedef void(*SwapPassUniforms)(uint64 camera_id, uint32 target_id, uint32 technique_id, uint64 uniforms_id);
typedef void(*SetUniform)(uint64 uniforms_id, unsigned index, float value);
typedef void(*DrawClusterBox)(const Id& id);
typedef void(*DrawPoint)(const Vector3& point, const Vector3& color);
typedef bool(*Play)(const Id& id, uint32 sound_id, float volume);

struct Commands {
    IsRelease is_release;
    ToggleProfileJobs toggle_profile_jobs;
    ToggleDrawBounds toggle_draw_bounds;
    ToggleDrawExtents toggle_draw_extents;
    DataId data_id;
    GetValue get_value;
    GetDynamicScale get_dynamic_scale;
    GetAspectRatio get_aspect_ratio;
    GetWindowSize get_window_size;
    GetGpuFrameDuration get_gpu_frame_duration;
    SetDynamicScale set_dynamic_scale;
    GetRotation get_rotation;
    GetPosition get_position;
    SetPosition set_position;
    SetRotation set_rotation;
    Pick pick;
    SwapSurface swap_surface;
    SwapUniforms swap_uniforms;
    SwapPassUniforms swap_pass_uniforms;
    SetUniform set_uniform;
    DrawClusterBox draw_cluster_box;
    DrawPoint draw_point;
    Play play;
};

typedef void(*InitFunc)(Commands& commands);
typedef void(*LogFunc)(const char* text);
typedef void(*MouseFunc)(MousePhase phase, Button button, float u, float v, float w);
typedef void(*KeyboardFunc)(KeyboardPhase phase, Key key);
typedef void(*UpdateFunc)(float timestep, unsigned window_witdh, unsigned window_height);

#if !defined(__APPLE__)
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT static
#endif
