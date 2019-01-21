
class Named : public NoCopy {
    String name;
    uint64 id = 0;

public:
    Named() {}
    Named(uint64 id) : id(id) {}
    Named(uint64 id, const String& name) : id(id), name(name) {}

    bool operator>(const Named& other) const { return id > other.id; }
    bool operator<(const Named& other) const { return id < other.id; }
    bool operator==(const Named& other) const { return id == other.id; }

    uint64 Id() const { return id; }
    const String& Name() const { return name; }
};

class Data : public Named {
public:
    static const size DynamicSize = 256; // TODO: Remove.

    enum class Type : uint16 {
        Cell = 0,
        Bank,
        Camera,
        Dictionary,
        Flags,
        Follow,
        Mesh,
        Script,
        Shader,
        Source,
        Surface,
        Uniforms,
        Count,
        Invalid
    };

    Data() {}
    Data(uint64 id, const String& name)
        : Named(id, name) {}

    static constexpr Type DataTypeFromId(uint64 id) { return (Type)((uint32)(id >> 32) & (((uint32)1 << 16) - 1)); }
    static constexpr uint64 CreateId(Type data_type, uint32 hash) { return ((uint64)data_type << 32) | (uint64)hash; }

    static Type DataTypeFromTypeName(const String& type_name) {
        if (type_name == "surface")         return Type::Surface;
        else if (type_name == "cell")       return Type::Cell;
        else if (type_name == "camera")     return Type::Camera;
        else if (type_name == "dictionary") return Type::Dictionary;
        else if (type_name == "flags")      return Type::Flags;
        else if (type_name == "follow")     return Type::Follow;
        else if (type_name == "mesh")       return Type::Mesh;
        else if (type_name == "script")     return Type::Script;
        else if (type_name == "shader")     return Type::Shader;
        else if (type_name == "bank")       return Type::Bank;
        else if (type_name == "source")     return Type::Source;
        else if (type_name == "uniforms")   return Type::Uniforms;
        else return Type::Invalid;
    }

    static Type DataTypeFromName(const String& name) {
        return DataTypeFromTypeName(name.SubString(name.FindLast('.') + 1));
    }

    static uint64 IdFromName(const String& name) {
        const Type data_type = DataTypeFromName(name);
        const uint32 hash = Hash::Fnv32(name.Data(), (int)name.Size());
        return CreateId(data_type, hash);
    }
};

class Instance {
public:
    Quaternion rotation;
    Vector3 position;
    float scale = 1.f;

    Instance() {}
    Instance(const Quaternion& rotation, const Vector3& position) : rotation(rotation), position(position) {}
};

class Batch : public Named {
public:
    static const unsigned InstanceMaxCount = 64;

private:
    Array<Instance, InstanceMaxCount> instances;
    Vector3 extents;
    Sphere sphere;

public:
    Batch() {}
    Batch(uint64 id, const String& name, const Vector3& extents)
        : Named(id, name), extents(extents) {}

    Array<Instance, InstanceMaxCount>& Instances() { return instances; }
    const Array<Instance, InstanceMaxCount>& Instances() const { return instances; }
    const Vector3& Extents() const { return extents; }
    const Sphere& Bounds() const { return sphere; }

    void ComputeBounds(Vector3& min, Vector3& max) {
        instances.ProcessIndex([&](auto& instance, unsigned index) {
            min = min.Minimum(instance.position - extents);
            max = max.Maximum(instance.position + extents);
        });
        new(&sphere) Sphere(min - extents, max + extents);
    }
};

typedef FixedArray<Instance, Batch::InstanceMaxCount> Instances;

class Cluster : public Named {
public:
    static const size DynamicSize = 512; // TODO: Remove.
    static const unsigned BatchMaxCount = 16;

private:
    Array<Batch, BatchMaxCount> batches;
    Sphere sphere;
    Quaternion rotation;
    Vector3 position;

public:
    Cluster() {}
    Cluster(uint64 id) : Named(id) {}
    Cluster(uint64 id, String& name, const Vector3& position, const Quaternion& rotation)
        : Named(id, name), position(position), rotation(rotation) {}

    Array<Batch, BatchMaxCount>& Batches() { return batches; }
    const Array<Batch, BatchMaxCount>& Batches() const { return batches; }
    const Sphere& Bounds() const { return sphere; }
    const Quaternion& Rotation() const { return rotation; }
    const Vector3& Position() const { return position; }

    Batch* Find(uint64 id) { // TODO: Remove.
        Batch* result = nullptr;
        batches.Process([&](auto& batch) {
            if (batch.Id() == id)
                result = &batch;
        });
        return result;
    }

    const Batch* ConstFind(uint64 id) const { // TODO: Remove.
        const Batch* result = nullptr;
        batches.ConstProcess([&](auto& batch) {
            if (batch.Id() == id)
                result = &batch;
        });
        return result;
    }

    const Batch* ConstFindIndex(uint64 id, unsigned& out_index) const { // TODO: Remove.
        const Batch* result = nullptr;
        batches.ConstProcessIndex([&](auto& batch, unsigned index) {
            if (batch.Id() == id) {
                result = &batch;
                out_index = index;
            }
        });
        return result;
    }

    void ComputeBounds() {
        Vector3 min(Math::Large), max(-Math::Large);
        batches.Process([&](auto& batch) {
            batch.ComputeBounds(min, max);
        });
        new(&sphere) Sphere(min, max);
    }
};

struct Cell : public Data {
protected:
    struct Table {
        enum Type {
            Script = 0,
            Follow,
            Source,
            Camera,
            Render,
            Count
        };

        uint32 size = 0;
        uint32 count = 0;
        uint32 offset = 0;

        Table() {}
        Table(uint32 size, uint32 count) : size(size), count(count) {}

        ::size TotalSize() const { return size * count; }
    };

    uint32 cluster_count = 0;
    uint32 cluster_offset = 0;
    FixedArray<Table, Table::Count> cluster_tables;

public:
    Cell(uint64 id, const String& name)
        : Data(id, name) {}
};

class Dictionary : public Data {
    static const unsigned EntryMaxCount = 64;

protected:
    struct Entry {
        uint32 hash = 0;
        float value = 0.0f;

        Entry() {}
        Entry(uint32 hash, float value)
            : hash(hash), value(value) {}
    };
    Array<Entry, EntryMaxCount> entries;

public:
    Dictionary(uint64 id, const String& name)
        : Data(id, name) {}

    float Find(uint32 hash) const {
        float value = 0.0f;
        entries.ConstProcess([&](const auto& entry) {
            if (entry.hash == hash)
                value = entry.value;
        });
        return value;
    }
};

class Follow : public Data {
protected:
    enum class Type : uint8 {
        None = 0,
        Carrot,
        Mirror, // X/Y plane at y = 0.
        Spline,
    };

    FixedArray<Vector3, Batch::InstanceMaxCount> initial_positions;
    FixedArray<float, Batch::InstanceMaxCount> weights;
    Vector3 tangent_begin;
    Vector3 tangent_end;
    float position_speed = 0.f;
    float rotation_speed = 0.f;
    uint64 followed_cluster_id = 0;
    uint64 followed_batch_id = 0;
    uint8 bucket_index = 0;
    Type type = Type::None;

public:
    Follow(uint64 id, const String& name)
        : Data(id, name) {}

    uint64 FollowedClusterId() const { return followed_cluster_id; }
    uint64 FollowedBatchId() const { return followed_batch_id; }
    uint8 GetBucketIndex() const { return bucket_index; }

    static const unsigned BucketMaxCount = 2;

    void Update(Batch& batch, const Batch& followed_batch) {
        switch (type) {
        case Type::Carrot: Carrot(batch, followed_batch); break;
        case Type::Mirror: Mirror(batch, followed_batch); break;
        case Type::Spline: Spline(batch, followed_batch); break;
        default: break;
        }
    }

    void Carrot(Batch& batch, const Batch& followed_batch) const {
        batch.Instances().ProcessIndex([&](auto& instance, unsigned index) {
            instance.rotation = instance.rotation.Slerp(followed_batch.Instances()[index].rotation, rotation_speed);
            instance.position = instance.position.Lerp(followed_batch.Instances()[index].position, position_speed);
        });
    }

    void Mirror(Batch& batch, const Batch& followed_batch) const {
        const auto& target_rotation = followed_batch.Instances()[0].rotation;
        const auto& target_position = followed_batch.Instances()[0].position;
        batch.Instances()[0].rotation = target_rotation.Conjugate();
        batch.Instances()[0].position = Vector3(target_position.x, -target_position.y, target_position.z);
    }

    void Spline(Batch& batch, const Batch& followed_batch) {
        batch.Instances().ProcessIndex([&](auto& instance, unsigned index) {
            const auto& target_rotation = followed_batch.Instances()[index].rotation;
            const auto& target_position = followed_batch.Instances()[index].position;
            if ((instance.rotation.SquareDistance(target_rotation) > 0.001f) || // TODO: Add threshold to data.
                (instance.position.SquareDistance(target_position) > 0.001f)) {
                instance.rotation = instance.rotation.Slerp(target_rotation, rotation_speed);
                instance.position = Vector3::CubicHermite(instance.position, target_position, tangent_begin, tangent_end, weights[index]);
                weights[index] = Math::Lerp(weights[index], 1.f, position_speed);
            }
            else {
                initial_positions[index] = target_position;
                instance.rotation = target_rotation;
                instance.position = target_position;
                weights[index] = 0.f;
            }
        });
    }
};

struct ClusterId {
    uint64 cluster_id = 0;

    ClusterId() {}
    ClusterId(uint64 cluster_id) : cluster_id(cluster_id) {}

    bool operator>(const ClusterId& other) const { return cluster_id > other.cluster_id; }
    bool operator<(const ClusterId& other) const { return cluster_id < other.cluster_id; }
    bool operator==(const ClusterId& other) const { return cluster_id == other.cluster_id; }
};

struct FollowCluster : public ClusterId {
    Array<uint64, Cluster::BatchMaxCount> follow_ids;

    FollowCluster() {}
    FollowCluster(uint64 cluster_id) : ClusterId(cluster_id) {}
    FollowCluster(const FollowCluster& other) { memcpy(this, &other, sizeof(FollowCluster)); }

    FollowCluster& operator=(const FollowCluster& other) { memcpy(this, &other, sizeof(FollowCluster)); return *this; }

    void AddFollowId(uint64 follow_id) { follow_ids.Add(follow_id); }
};

class Source : public Data {
public:
    Source(uint64 id, const String& name)
        : Data(id, name) {}
};

struct SourceCluster : public ClusterId {
    uint64 bank_id = 0;
    Array<uint64, Cluster::BatchMaxCount> source_ids;

    SourceCluster() {}
    SourceCluster(uint64 cluster_id, uint64 bank_id) : ClusterId(cluster_id), bank_id(bank_id) {}
    SourceCluster(const SourceCluster& other) { memcpy(this, &other, sizeof(SourceCluster)); }

    SourceCluster& operator=(const SourceCluster& other) { memcpy(this, &other, sizeof(SourceCluster)); return *this; }

    void AddSourceId(uint64 source_id) { source_ids.Add(source_id); }
};

class Flags : public Data {
protected:
    uint32 flags = 0;

public:
    Flags(uint64 id, const String& name)
        : Data(id, name) {}

    bool Check(const uint32 include_flags, const uint32 exclude_flags) const {
        if ((flags & include_flags) != include_flags)
            return false;
        if ((flags & exclude_flags) > 0)
            return false;
        return true;
    }

    uint32 Value() const { return flags; }
};

class Uniforms : public Data {
public:
    static const unsigned UniformMaxCount = 8;

protected:
    FixedArray<Vector4, UniformMaxCount> uniforms;

public:
    Uniforms(uint64 id, const String& name)
        : Data(id, name) {}

    Vector4* Values() { return uniforms.Values(); }
    const Vector4* Values() const { return uniforms.Values(); }
};

class Camera : public Data {
public:
    static const unsigned PassMaxCount = 4;
    static const unsigned TargetMaxCount = 32;
    static const unsigned SurfaceMaxCount = 8;
    static const unsigned ColorAttachmentMaxCount = 4;

    struct Pass {
        Array<uint64, SurfaceMaxCount> auto_surface_ids;
        uint64 auto_shader_id = 0;
        uint64 auto_mesh_id = 0;
        uint64 auto_uniforms_id = 0;
        uint32 technique_id = 0;
        uint32 include_flags = 0;
        uint32 exclude_flags = 0;
        uint32 profile = 0;
    };

    struct Target {
        Array<Pass, PassMaxCount> passes;
        Array<uint64, ColorAttachmentMaxCount> color_attachment_ids;
        uint64 depth_stencil_attachment_id = 0;
        uint32 target_id = 0;
        bool clear_color = true;
        bool clear_depth = true;
        bool last = false;
    };

    struct Uniforms {
        Matrix view;
        Matrix view_inverse;
        Matrix proj;
        Matrix proj_inverse;
        Matrix viewproj;
        Matrix viewproj_inverse;
        Vector4 position;
        Vector4 direction;
    };

protected:
    Array<Target, TargetMaxCount> targets;
    float priority = 0.f;
    float fov = 0.f;
    float persp_near = 0.f;
    float persp_far = 0.f;
    float ortho_width = 0.f;
    float ortho_height = 0.f;

public:
    Camera(uint64 id, const String& name)
        : Data(id, name) {}

    float Priority() const { return priority; }
    Array<Target, TargetMaxCount>& Targets() { return targets; }
    const Array<Target, TargetMaxCount>& Targets() const { return targets; }

    void Update(Uniforms& uniforms, const Vector3& position, const Quaternion& rotation, const unsigned window_width, const unsigned window_height) const {
        const float aspect_ratio = (float)window_width / (float)window_height;
        Matrix::LookAtLH(position, rotation.At(), rotation.Up(), uniforms.view, uniforms.view_inverse);
        if ((ortho_width > 0.f) && (ortho_height > 0.f))
            Matrix::OrthoLH(ortho_width * aspect_ratio, ortho_height, persp_near, persp_far, uniforms.proj, uniforms.proj_inverse);
        else
            Matrix::PerspectiveFovLH(fov, aspect_ratio, persp_near, persp_far, uniforms.proj, uniforms.proj_inverse);
        uniforms.viewproj = uniforms.view * uniforms.proj;
        uniforms.viewproj_inverse = uniforms.proj_inverse * uniforms.view_inverse;
        uniforms.position = Vector4(position, 1.f);
        uniforms.direction = Vector4(rotation.At(), 0.f);
    }
};

struct CameraCluster : public ClusterId {
    uint64 camera_id = 0;

    CameraCluster() {}
    CameraCluster(uint64 cluster_id, uint64 camera_id) : ClusterId(cluster_id), camera_id(camera_id) {}
    CameraCluster(const CameraCluster& other) { memcpy(this, &other, sizeof(CameraCluster)); }

    CameraCluster& operator=(const CameraCluster& other) { memcpy(this, &other, sizeof(CameraCluster)); return *this; }
};

class Bank : public Data {
public:
    static const size SoundFormatMaxSize = 64;

protected:
    static const unsigned SoundMaxCount = 16;

    class Sound {
        uint8 raw_format[SoundFormatMaxSize];
        uint32 id = 0;
        uint32 length = 0;
        uint32 offset = 0;

    public:
        Sound() {}
        Sound(uint32 id, const char* format, size format_size, size offset, size length)
            : id(id), offset((uint32)offset), length((uint32)length) {
            memcpy(raw_format, format, format_size);
        }

        const char* RawFormat() const { return (char*)raw_format; }
        uint32 Id() const { return id; }
        uint32 Offset() const { return offset; }
        uint32 Length() const { return length; }
    };

    Array<Sound, SoundMaxCount> sounds;

public:
    Bank(uint64 id, const String& name)
        : Data(id, name) {}

    const Sound* FindSound(uint32 sound_id) const {
        const Sound* result = nullptr;
        sounds.ConstFind([&](const auto& sound) {
            if (sound.Id() == sound_id) {
                result = &sound;
                return true;
            }
            return false;
        });
        return result;
    }
};

class Script : public Data {
public:
    Script(uint64 id, const String& name)
        : Data(id, name) {}
};

struct ScriptCluster : public ClusterId {
    uint64 script_id = 0;

    ScriptCluster(uint64 cluster_id, uint64 script_id)
        : ClusterId(cluster_id), script_id(script_id) {}
    ScriptCluster(const ScriptCluster& other) { memcpy(this, &other, sizeof(ScriptCluster)); }

    ScriptCluster& operator=(const ScriptCluster& other) { memcpy(this, &other, sizeof(ScriptCluster)); return *this; }
};

class Mesh : public Data {
 protected:
    class Attribute {
    public:
        enum class Type : uint16 {
            None = 0,
            U8,
            S8,
            U16,
            S16,
            F16,
            U32,
            S32,
            F32,
        };

        void Fill(Type type, uint8 stride, uint8 offset, uint32 offset_from_base) {
            this->offset_from_base = offset_from_base;
            this->type = type;
            this->stride = stride;
            this->offset = offset;
        }

        uint32 Offset() const { return offset_from_base + offset; }
        unsigned Stride() const { return stride; }
        Type AttributeType() const { return type; }

    private:
        uint32 offset_from_base = 0;
        Type type = Type::None;
        uint8 stride = 0;
        uint8 offset = 0;
    };

    static const unsigned UVSetMaxCount = 4;
    static const unsigned ColorSetMaxCount = 4;

    Attribute vertices;
    Attribute normals;
    Array<Attribute, UVSetMaxCount> uv_sets;
    Array<Attribute, ColorSetMaxCount> color_sets;
    Attribute indices;
    uint32 vertex_count = 0;
    uint32 index_count = 0;

public:
    Mesh(uint64 id, const String& name)
        : Data(id, name) {}
};

enum class PixelFormat : uint8 {
    None = 0,
    D16,
    D32F,
    R8,
    R16F,
    R32F,
    RG8,
    RGB8,
    RGB11F,
    RGBA8,
    BGRA8,
    RGBA10,
    RG16F,
    RGB32F,
    RGBA16F,
    RGBA32F,
    BC1,
    BC2,
    BC3,
};

enum class Dimension : uint8 {
    None = 0,
    Tex1D,
    Tex2D,
    Tex3D,
};

class Shader : public Data {
public:
    static const unsigned TechniqueMaxCount = 16;

    Shader(uint64 id, const String& name)
        : Data(id, name) {}

    unsigned FindTechnique(const uint32 technique_id) const {
        unsigned result = (unsigned)-1;
        techniques.ConstProcessIndex([&](const auto& technique, unsigned index) {
            if (technique.ID() == technique_id)
                result = index;
        });
        return result;
    }

    static const unsigned InputMaxCount = 4;
    static const unsigned DescriptorMaxCount = 8;
    static const unsigned SamplerMaxCount = 8;
    static const size FunctionNameMaxSize = 64;

    enum class State : uint16 {
        None = 0,
        DepthTest = 1 << 0,
        DepthWrite = 1 << 1,
        FrontFaceCull = 1 << 2,
        BackFaceCull = 1 << 3,
        Alpha = 1 << 4,
        AlphaPreMultiplied = 1 << 5,
        AlphaPostMultiplied = 1 << 6,
        AlphaAdditive = 1 << 7,
        AlphaMultiplicative = 1 << 8
    };

    struct Input {
        enum class Semantic : uint8 {
            None = 0,
            Position,
            Texcoord,
            Normal,
            Color,
        };

        enum class Format : uint8 {
            None = 0,
            char4,
            uchar4,
            half2,
            float3,
        };

        Semantic semantic = Semantic::None;
        Format format = Format::None;
        uint8 index = 0;
    };

    struct Sampler {
        enum class Filter : uint8 {
            None = 0,
            MinMagMipPoint,
            MinMagMipLinear,
            Anisotropic,
            ComparisonMinMagMipLinear,
            ComparisonAnisotropic,
        };

        enum class AddressMode : uint8 {
            None = 0,
            Wrap,
            Mirror,
            Clamp,
            Border,
            MirrorOnce,
        };

        enum class Comparison : uint8 {
            None = 0,
            Never,
            Less,
            Equal,
            LessEqual,
            Greater,
            NotEqual,
            GreaterEqual,
            Always,
        };

        Filter filter = Filter::None;
        AddressMode address_mode = AddressMode::Wrap;
        Comparison comparison = Comparison::Always;
    };

    struct Descriptor {
        enum class Type : uint8 {
            None = 0,
            Texture,
            Buffer,
        };

        Type type = Type::None;
    };

    struct Binary {
        uint32 offset_from_base = 0;
        uint32 size = 0;
    };

    class Technique : public NoCopy {
    public:
        Binary vertex_binary;
        Binary pixel_binary;
        Binary compute_binary;
        uint32 technique_id = 0;
        BitFlags<State> state_flags = State::None;
        float depth_bias = 0.0f;
        float slope_scale_depth_bias = 0.0f;
        PixelFormat depth_attachment_pixel_format = PixelFormat::None;
        Array<PixelFormat, Camera::ColorAttachmentMaxCount> color_attachment_pixel_formats;
        String vertex_function_name;
        String pixel_function_name;

        uint32 ID() const { return technique_id; }
    };

    Array<Technique, TechniqueMaxCount> techniques;
    Array<Input, InputMaxCount> inputs;
    Array<Descriptor, DescriptorMaxCount> descriptors;
    Array<Sampler, SamplerMaxCount> samplers;
};

class Surface : public Data {
protected:
    Vector4 clear_color;
    Vector2 sizes;
    float resize = 0.f;
    uint16 width = 0;
    uint16 height = 0;
    uint16 depth = 0;
    uint8 bits_per_pixel = 0;
    uint8 slice_count = 0;
    uint8 mip_count = 0;
    uint8 is_texture = 0;
    uint8 is_cube_map = 0;
    uint8 is_default = 0;
    uint8 is_depth_stencil = 0;
    uint8 do_clear = 0;
    Dimension dimension = Dimension::None;
    PixelFormat pixel_format = PixelFormat::None;

public:
    Surface(uint64 id, const String& name)
        : Data(id, name) {}

    unsigned Width() const { return width; }
    unsigned Height() const { return height; }
    PixelFormat Format() const { return pixel_format; }
};

struct RenderCluster : public ClusterId {
    uint64 flags_id = 0;
    uint64 shader_id = 0;
    Array<uint64, Camera::SurfaceMaxCount> surface_ids;
    bool has_camera = false; // TODO: Remove.
    Array<uint64, Cluster::BatchMaxCount> mesh_ids;
    Array<uint64, Cluster::BatchMaxCount> uniforms_ids;

    RenderCluster() {}
    RenderCluster(uint64 cluster_id, uint64 flags_id, uint64 shader_id, Array<uint64, Camera::SurfaceMaxCount>& surface_ids, bool has_camera, Array<uint64, Cluster::BatchMaxCount>& mesh_ids, Array<uint64, Cluster::BatchMaxCount>& uniforms_ids)
        : ClusterId(cluster_id), flags_id(flags_id), shader_id(shader_id), surface_ids(surface_ids), has_camera(has_camera), mesh_ids(mesh_ids), uniforms_ids(uniforms_ids) {}
    RenderCluster(const RenderCluster& other) { memcpy(this, &other, sizeof(RenderCluster)); }

    RenderCluster& operator=(const RenderCluster& other) { memcpy(this, &other, sizeof(RenderCluster)); return *this; }
};


class Bundle : public NoCopy {
public:
    static const String Name() { return "data.bundle"; }

    struct Resource {
        uint64 data_id = 0;
        uint32 offset = 0;
        uint32 length = 0;

        Resource() {}
        Resource(uint64 data_id) : data_id(data_id) {}
        Resource(uint64 data_id, size offset, size size) : data_id(data_id), offset((uint32)offset), length((uint32)size) {}

        bool operator<(const Resource& other) const { return data_id < other.data_id; }
        bool operator==(const Resource& other) const { return data_id == other.data_id; }
    };

    class Asset {
        const uint8* mem = nullptr;
        size size = 0;

    public:
        Asset() {}
        Asset(const uint8* mem, ::size size) : mem(mem), size(size) {}

        operator bool() const { return mem != nullptr; }

        const uint8* Mem() const { return mem; }
        ::size Size() const { return size; }
    };

private:
    ReadCopyFile file;
    ProxyArray<Resource> headers;
    ProxyArray<Resource> datas;

public:
    Bundle() : file(Name()) {
        const auto* in = (uint8*)file.Pointer();
        const unsigned header_count = *(uint32*)in;
        in += sizeof(uint32);
        const unsigned data_count = *(uint32*)in;
        in += sizeof(uint32);
        new(&headers) ProxyArray<Resource>((Resource*)in, header_count);
        in += header_count * sizeof(Resource);
        new(&datas) ProxyArray<Resource>((Resource*)in, data_count);
        in += data_count * sizeof(Resource);
    }

    Asset FindData(uint64 data_id) const {
        const auto* resource = datas.ConstBinaryFind(data_id);
        return resource ? Asset((uint8*)file.Pointer() + resource->offset, resource->length) : Asset(nullptr, 0);
    }

    template <typename T> T* Find(uint64 data_id) {
        const auto* resource = headers.ConstBinaryFind(data_id);
        return resource ? (T*)((uint8*)file.Pointer() + resource->offset) : nullptr;
    }

    template<typename F> void Process(F func) {
        headers.Process([&](auto& resource) {
            func(*(Data*)((uint8*)file.Pointer() + resource.offset));
        });
    }
};