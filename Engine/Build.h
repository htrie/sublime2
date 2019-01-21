
String RootPath = "";
const String AssetPath() { return String("Assets/"); }
const String CachePath() { return RootPath + String("Cache/"); }

static const String AssetFilename(const String& name) { return AssetPath() + name + ".xml"; }
static const String HeaderFilename(const String& name) { return name + ".header"; }
static const String DataFilename(const String& name) { return name + ".data"; }
static const String CacheHeaderFilename(const String& name) { return CachePath() + HeaderFilename(name); }
static const String CacheDataFilename(const String& name) { return CachePath() + DataFilename(name); }

class Alloc { // TODO: Remove.
    void* mem = nullptr;
    size size = 0;

public:
    Alloc() {}
    Alloc(::size size) : mem(Memory::Malloc(size)), size(size) {}
    Alloc(void* data, ::size size) : mem(Memory::Malloc(size)), size(size) { memcpy(mem, data, size); }
    Alloc(Alloc& other) : mem(other.mem), size(other.size) { other.mem = nullptr; other.size = 0; }
    ~Alloc() { Memory::Free(mem, size); }

    Alloc& operator=(Alloc& other) { mem = other.mem; size = other.size; other.mem = nullptr; other.size = 0; return *this; }

    void* Pointer() const { return mem; }
    ::size Size() const { return size; }
};

class BatchBuild {
public:
    static const unsigned DataIndexMaxCount = 4;

private:
    Array<uint8, DataIndexMaxCount> data_indices;

public:
    Array<uint8, DataIndexMaxCount>& DataIndices() { return data_indices; }
    const Array<uint8, DataIndexMaxCount>& DataIndices() const { return data_indices; }
};

class ClusterBuild : public Cluster {
public:
    static const unsigned DataMaxCount = 16;

private:
    Array<BatchBuild, BatchMaxCount> batches_build;
    Array<uint8, DataMaxCount> data_indices;
    Array<uint64, DataMaxCount> data_ids;

public:
    ClusterBuild() {}
    ClusterBuild(uint64 id, String& name, const Vector3& position, const Quaternion& rotation)
        : Cluster(id, name, position, rotation) {}
    ClusterBuild(const ClusterBuild& other) { memcpy(this, &other, sizeof(ClusterBuild)); }

    ClusterBuild& operator=(const ClusterBuild& other) { memcpy(this, &other, sizeof(ClusterBuild)); return *this; }

    Array<BatchBuild, BatchMaxCount>& BatchesBuild() { return batches_build; }
    const Array<BatchBuild, BatchMaxCount>& BatchesBuild() const { return batches_build; }
    Array<uint8, DataMaxCount>& DataIndices() { return data_indices; }
    const Array<uint8, DataMaxCount>& DataIndices() const { return data_indices; }
    Array<uint64, DataMaxCount>& DataIds() { return data_ids; }
    const Array<uint64, DataMaxCount>& DataIds() const { return data_ids; }

    unsigned FindData(uint64 id) { // TODO: Remove.
        unsigned result = (unsigned)-1;
        data_ids.ProcessIndex([&](auto& _id, unsigned index) {
            if (id == _id)
                result = index;
        });
        return result;
    }

    uint8 AddData(uint64 id) {
        data_ids.Add(id);
        return data_ids.UsedCount() - 1;
    }
};

struct CellBuild : public Cell {
    static const unsigned ClusterMaxCount = 256; // TODO: Remove.

    CellBuild(uint64 id, const String& name) : Cell(id, name) {
        ReadOnlyFile xml_file(AssetFilename(name));
        XML::Doc doc((char*)xml_file.Pointer());
        auto root = doc.FirstNode();
        
        Alloc alloc(sizeof(Array<ClusterBuild, ClusterMaxCount>));
        auto& clusters_build = *(Array<ClusterBuild, ClusterMaxCount>*)alloc.Pointer();
        ReadClusters(root, clusters_build);

        cluster_count = clusters_build.UsedCount();

        FixedArray<Alloc, Table::Count> allocs;
        allocs[Table::Script] = Alloc(sizeof(Array<ScriptCluster, ClusterMaxCount>));
        allocs[Table::Follow] = Alloc(sizeof(Array<FollowCluster, ClusterMaxCount>));
        allocs[Table::Source] = Alloc(sizeof(Array<SourceCluster, ClusterMaxCount>));
        allocs[Table::Camera] = Alloc(sizeof(Array<CameraCluster, ClusterMaxCount>));
        allocs[Table::Render] = Alloc(sizeof(Array<RenderCluster, ClusterMaxCount>));

        auto& script_clusters = *(Array<ScriptCluster, ClusterMaxCount>*)allocs[Table::Script].Pointer();
        auto& follow_clusters = *(Array<FollowCluster, ClusterMaxCount>*)allocs[Table::Follow].Pointer();
        auto& source_clusters = *(Array<SourceCluster, ClusterMaxCount>*)allocs[Table::Source].Pointer();
        auto& camera_clusters = *(Array<CameraCluster, ClusterMaxCount>*)allocs[Table::Camera].Pointer();
        auto& render_clusters = *(Array<RenderCluster, ClusterMaxCount>*)allocs[Table::Render].Pointer();

        ParseClusters(clusters_build, script_clusters, follow_clusters, source_clusters, camera_clusters, render_clusters);

        cluster_tables[Table::Script] = Table((uint32)(sizeof(ScriptCluster) + Cluster::DynamicSize), script_clusters.UsedCount());
        cluster_tables[Table::Follow] = Table((uint32)(sizeof(FollowCluster) + Cluster::DynamicSize), follow_clusters.UsedCount());
        cluster_tables[Table::Source] = Table((uint32)(sizeof(SourceCluster) + Cluster::DynamicSize), source_clusters.UsedCount());
        cluster_tables[Table::Camera] = Table((uint32)(sizeof(CameraCluster) + Cluster::DynamicSize), camera_clusters.UsedCount());
        cluster_tables[Table::Render] = Table((uint32)(sizeof(RenderCluster) + Cluster::DynamicSize), render_clusters.UsedCount());

        WriteClusters(clusters_build, script_clusters, follow_clusters, source_clusters, camera_clusters, render_clusters);
    }

    void WriteClusters(const Array<ClusterBuild, ClusterMaxCount>& clusters_build,
            const Array<ScriptCluster, ClusterMaxCount>& script_clusters,
            const Array<FollowCluster, ClusterMaxCount>& follow_clusters,
            const Array<SourceCluster, ClusterMaxCount>& source_clusters,
            const Array<CameraCluster, ClusterMaxCount>& camera_clusters,
            const Array<RenderCluster, ClusterMaxCount>& render_clusters) {
        size total_size = sizeof(Cluster) * clusters_build.UsedCount();
        cluster_tables.ConstProcess([&](auto& table) {
            total_size += table.TotalSize();
        });
        WriteOnlyFile data_file(CacheDataFilename(Name()), total_size);
        auto* out = (uint8*)data_file.Pointer();
        size offset = 0;

        cluster_offset = (uint32)offset;
        clusters_build.ConstProcess([&](auto& cluster_build) {
            memcpy(&out[offset], &cluster_build, sizeof(Cluster));
            offset += sizeof(Cluster);
        });

        WriteClusterTable(script_clusters, cluster_tables[Table::Script], out, offset);
        WriteClusterTable(follow_clusters, cluster_tables[Table::Follow], out, offset);
        WriteClusterTable(source_clusters, cluster_tables[Table::Source], out, offset);
        WriteClusterTable(camera_clusters, cluster_tables[Table::Camera], out, offset);
        WriteClusterTable(render_clusters, cluster_tables[Table::Render], out, offset);
    }

    template <typename C> void WriteClusterTable(const Array<C, ClusterMaxCount>& clusters, Table& table, uint8* out, size& offset) {
        table.offset = (uint32)offset;
        clusters.ConstProcess([&](auto& cluster) {
            memcpy(&out[offset], &cluster, sizeof(C));
            offset += sizeof(C);
            memset(&out[offset], 0, Cluster::DynamicSize);
            offset += Cluster::DynamicSize;
        });
    }

    void ParseClusters(const Array<ClusterBuild, ClusterMaxCount>& clusters_build,
            Array<ScriptCluster, ClusterMaxCount>& script_clusters, 
            Array<FollowCluster, ClusterMaxCount>& follow_clusters,
            Array<SourceCluster, ClusterMaxCount>& source_clusters,
            Array<CameraCluster, ClusterMaxCount>& camera_clusters,
            Array<RenderCluster, ClusterMaxCount>& render_clusters) {
        clusters_build.ConstProcess([&](auto& cluster_build) {
            uint64 bank_id = 0;
            uint64 camera_id = 0;
            uint64 flags_id = 0;
            uint64 shader_id = 0;
            uint64 script_id = 0;
            Array<uint64, Camera::SurfaceMaxCount> surface_ids;

            cluster_build.DataIndices().ConstProcess([&](const auto& data_index) {
                const auto data_id = cluster_build.DataIds()[data_index];
                switch (Data::DataTypeFromId(data_id)) {
                case Data::Type::Bank: bank_id = data_id; break;
                case Data::Type::Camera: camera_id = data_id; break;
                case Data::Type::Flags: flags_id = data_id; break;
                case Data::Type::Shader: shader_id = data_id; break;
                case Data::Type::Surface: surface_ids.Add(data_id); break;
                case Data::Type::Script: script_id = data_id; break;
                default: break;
                }
            });

            Array<uint64, Cluster::BatchMaxCount> source_ids;
            Array<uint64, Cluster::BatchMaxCount> follow_ids;
            Array<uint64, Cluster::BatchMaxCount> mesh_ids;
            Array<uint64, Cluster::BatchMaxCount> uniforms_ids;

            cluster_build.BatchesBuild().ConstProcess([&](auto& batch_build) {
                uint64 uniforms_id = 0;
                uint64 mesh_id = 0;
                batch_build.DataIndices().ConstProcess([&](auto& data_index) {
                    const auto data_id = cluster_build.DataIds()[data_index];
                    switch (Data::DataTypeFromId(data_id)) {
                    case Data::Type::Follow: follow_ids.Add(data_id); break;
                    case Data::Type::Mesh: mesh_id = data_id; break;
                    case Data::Type::Source: source_ids.Add(data_id); break;
                    case Data::Type::Uniforms: uniforms_id = data_id; break;
                    default: break;
                    }
                });
                if (shader_id) {
                    mesh_ids.Add(mesh_id);
                    uniforms_ids.Add(uniforms_id);
                }
            });

            if (script_id) {
                script_clusters.Add(cluster_build.Id(), script_id);
            }

            if (follow_ids.UsedCount() > 0) {
                auto& follow_cluster = follow_clusters.Add(cluster_build.Id());
                follow_ids.ConstProcess([&](const auto& follow_id) {
                    follow_cluster.AddFollowId(follow_id);
                });
            }

            if (source_ids.UsedCount() > 0) {
                auto& source_cluster = source_clusters.Add(cluster_build.Id(), bank_id);
                source_ids.ConstProcess([&](const auto& source_id) {
                    source_cluster.AddSourceId(source_id);
                });
            }

            if (camera_id) {
                camera_clusters.Add(cluster_build.Id(), camera_id);
            }

            if (shader_id) {
                render_clusters.Add(cluster_build.Id(), flags_id, shader_id, surface_ids, camera_id != 0, mesh_ids, uniforms_ids);
            }
        });
        script_clusters.Sort();
        follow_clusters.Sort();
        source_clusters.Sort();
        camera_clusters.Sort();
        render_clusters.Sort();
    }

    void ReadClusters(const XML::Node* root, Array<ClusterBuild, ClusterMaxCount>& clusters_build) {
        auto* node = root->FirstNode("cluster");
        while (node) {
            auto cluster_name = node->Text("name");
            const uint64 cluster_id = node->Hash64("name");
            Vector3 position;
            node->Vec3(&position.x, "position", 0.f);
            Quaternion rotation;
            node->Quat(&rotation.x, "rotation");
            rotation = rotation.Normalize();

            auto& cluster = clusters_build.Add();
            new(&cluster) ClusterBuild(cluster_id, cluster_name, position, rotation);
            ReadClusterDatas(node, cluster);
            ReadClusterBatches(node, cluster);
            cluster.ComputeBounds();
            node = node->NextSibling("cluster");
        }
        clusters_build.Sort();
    }

    void ReadClusterBatches(const XML::Node* parent, ClusterBuild& cluster_build) {
        auto node = parent->FirstNode("batch");
        while (node) {
            auto name = node->Text("name");
            const uint64 id = node->Hash64("name");
            Vector3 extents;
            node->Vec3(&extents.x, "extents", 1.f);
            ReadBatchInstances(node, cluster_build.Batches().Add(id, name, extents));
            ReadBatchDatas(node, cluster_build, cluster_build.BatchesBuild().Add());
            node = node->NextSibling("batch");
        }
        if (cluster_build.Batches().UsedCount() == 0) throw Exception("Cluster cannot have 0 batch");
    }

    void ReadClusterDatas(const XML::Node* parent, ClusterBuild& cluster_build) {
        auto node = parent->FirstNode();
        while (node) {
            if (String(node->Name(), node->NameSize()) != "data") {
                node = node->NextSibling();
                continue;
            }
            unsigned data_index = ReadData(node, cluster_build);
            cluster_build.DataIndices().Add(data_index);
            node = node->NextSibling();
        }
    }

    void ReadBatchDatas(const XML::Node* parent, ClusterBuild& cluster_build, BatchBuild& batch_build) {
        auto node = parent->FirstNode();
        while (node) {
            if (String(node->Name(), node->NameSize()) != "data") {
                node = node->NextSibling();
                continue;
            }
            unsigned data_index = ReadData(node, cluster_build);
            switch (Data::DataTypeFromId(cluster_build.DataIds()[data_index])) {
            case Data::Type::Surface: throw Exception("Cannot have surface data in batch.");
            case Data::Type::Camera: throw Exception("Cannot have camera data in batch.");
            case Data::Type::Flags: throw Exception("Cannot have flags data in batch.");
            case Data::Type::Script: throw Exception("Cannot have script data in batch.");
            case Data::Type::Shader: throw Exception("Cannot have shader data in batch.");
            case Data::Type::Bank: throw Exception("Cannot have bank data in batch.");
            default: break;
            }
            batch_build.DataIndices().Add(data_index);
            node = node->NextSibling();
        }
    }

    void ReadBatchInstances(const XML::Node* parent, Batch& batch) {
        auto node = parent->FirstNode("node");
        while (node) {
            Vector3 position;
            node->Vec3(&position.x, "position", 0.f);
            Quaternion rotation;
            node->Quat(&rotation.x, "rotation");
            rotation = rotation.Normalize();
            batch.Instances().Add(rotation, position);
            node = node->NextSibling("node");
        }
        if (batch.Instances().UsedCount() == 0) throw Exception("Batch cannot have 0 node");
    }

    unsigned ReadData(const XML::Node* node, ClusterBuild& cluster_build) {
        auto name = node->Text("name");
        const uint64 id = IdFromName(name);
        unsigned data_index = cluster_build.FindData(id);
        if (data_index == (unsigned)-1) {
            data_index = cluster_build.AddData(id);
        }
        return data_index;
    }
};

struct DictionaryBuild : public Dictionary {
    DictionaryBuild(uint64 id, const String& name) : Dictionary(id, name) {
        ReadOnlyFile xml_file(AssetFilename(name));
        XML::Doc doc((char*)xml_file.Pointer());
        auto root = doc.FirstNode();

        auto node = root->FirstNode("entry");
        while (node) {
            auto name = node->Hash32("name");
            auto value = node->Float("value", 0.0f);
            entries.Add(name, value);
            node = node->NextSibling("entry");
        }
    }
};

struct FollowBuild : public Follow {
    FollowBuild(uint64 id, const String& name) : Follow(id, name) {
        ReadOnlyFile xml_file(AssetFilename(name));
        XML::Doc doc((char*)xml_file.Pointer());
        auto root = doc.FirstNode();

        root->Vec3(&tangent_begin.x, "tangent_begin", 0.f);
        root->Vec3(&tangent_end.x, "tangent_end", 0.f);
        position_speed = root->Float("pos_speed", 1.f);
        rotation_speed = root->Float("rot_speed", 1.f);
        followed_cluster_id = root->Hash64("cluster");
        followed_batch_id = root->Hash64("batch");
        bucket_index = root->Int("bucket", 0);
        if (bucket_index >= Follow::BucketMaxCount)
            throw Exception("Invalid bucket index");
        type = ReadType(root);
    }

    Type ReadType(const XML::Node* parent) {
        auto type = parent->Text("type");
        if (type == "carrot") return Type::Carrot;
        else if (type == "mirror") return Type::Mirror;
        else if (type == "spline") return Type::Spline;
        return Type::None;
    }
};

struct SourceBuild : public Source {
    SourceBuild(uint64 id, const String& name) : Source(id, name) {
        ReadOnlyFile xml_file(AssetFilename(name));
        XML::Doc doc((char*)xml_file.Pointer());
        //auto root = doc.FirstNode();
    }
};

struct FlagsBuild : public Flags {
    FlagsBuild(uint64 id, const String& name) : Flags(id, name) {
        ReadOnlyFile xml_file(AssetFilename(name));
        XML::Doc doc((char*)xml_file.Pointer());
        auto root = doc.FirstNode();

        auto node = root->FirstNode("flag");
        while (node) {
            flags |= node->Int("value", 0);
            node = node->NextSibling("flag");
        }
    }
};

struct UniformsBuild : public Uniforms {
    UniformsBuild(uint64 id, const String& name) : Uniforms(id, name) {
        ReadOnlyFile xml_file(AssetFilename(name));
        XML::Doc doc((char*)xml_file.Pointer());
        auto root = doc.FirstNode();

        unsigned uniform_count = 0;
        float* values = &uniforms[0].x;
        auto node = root->FirstNode("uniform");
        while (node) {
            if (uniform_count >= Uniforms::UniformMaxCount * 4)
                throw Exception("Too many uniforms");
            const auto type = node->Text("type");
            if (type == "color") {
                node->Vec4(&values[uniform_count], "value", 0.f);
                uniform_count += 4;
            }
            else {
                values[uniform_count++] = node->Float("value", 0.f);
            }
            node = node->NextSibling("uniform");
        }
    }
};

struct CameraBuild : public Camera {
    CameraBuild(uint64 id, const String& name) : Camera(id, name) {
        ReadOnlyFile xml_file(AssetFilename(name));
        XML::Doc doc((char*)xml_file.Pointer());
        auto root = doc.FirstNode();

        fov = Math::Clamp(root->Float("fov", 90.f), 0.f, 180.f);
        priority = root->Float("priority", 0.f);
        persp_near = root->Float("near", 0.1f);
        persp_far = root->Float("far", 1000.f);
        root->Vec2(&ortho_width, "ortho", 0.f);
        ReadTargets(root);
    }

    void ReadPasses(Target& target, const XML::Node* parent) {
        auto node = parent->FirstNode("pass");
        while (node) {
            Pass& pass = target.passes.Add();
            ReadShader(pass, node);
            ReadMesh(pass, node);
            ReadSurfaces(pass, node);
            ReadUniforms(pass, node);
            pass.technique_id = node->Hash32("technique");
            node->Color(pass.profile, "profile", 0);
            ReadIncludes(pass, node);
            ReadExcludes(pass, node);
            node = node->NextSibling("pass");
        }
    }

    void ReadShader(Pass& pass, const XML::Node* parent) {
        auto node = parent->FirstNode("shader");
        if (node) {
            pass.auto_shader_id = IdFromName(node->Text("name"));
        }
    }

    void ReadMesh(Pass& pass, const XML::Node* parent) {
        auto node = parent->FirstNode("mesh");
        if (node) {
            pass.auto_mesh_id = IdFromName(node->Text("name"));
        }
    }

    void ReadSurfaces(Pass& pass, const XML::Node* parent) {
        auto node = parent->FirstNode("surface");
        while (node) {
            pass.auto_surface_ids.Add(IdFromName(node->Text("name")));
            node = node->NextSibling("surface");
        }
    }

    void ReadUniforms(Pass& pass, const XML::Node* parent) {
        auto node = parent->FirstNode("uniforms");
        if (node) {
            pass.auto_uniforms_id = IdFromName(node->Text("name"));
        }
    }

    void ReadIncludes(Pass& pass, const XML::Node* parent) {
        auto node = parent->FirstNode("include");
        while (node) {
            pass.include_flags |= node->Int("flag", 0);
            node = node->NextSibling("include");
        }
    }

    void ReadExcludes(Pass& pass, const XML::Node* parent) {
        auto node = parent->FirstNode("exclude");
        while (node) {
            pass.exclude_flags |= node->Int("flag", 0);
            node = node->NextSibling("exclude");
        }
    }

    void ReadTargets(const XML::Node* parent) {
        auto node = parent->FirstNode("target");
        while (node) {
            Target& target = targets.Add();
            target.target_id = node->Hash32("name");
            target.clear_color = node->Bool("clear_color", true);
            target.clear_depth = node->Bool("clear_depth", true);
            target.last = node->Bool("last", false);
            ReadColorAttachments(target, node);
            ReadDepthStencilAttachment(target, node);
            ReadPasses(target, node);
            if ((target.color_attachment_ids.UsedCount() == 0) && (target.depth_stencil_attachment_id == 0))
                throw Exception("Should have at least 1 attachment");
            node = node->NextSibling("target");
        }
    }

    void ReadColorAttachments(Target& target, const XML::Node* parent) {
        auto node = parent->FirstNode("color");
        while (node) {
            auto name = node->Text("name");
            if (DataTypeFromName(name) != Data::Type::Surface)
                throw Exception("Attachment name must have an surface type");
            target.color_attachment_ids.Add(IdFromName(name));
            node = node->NextSibling("color");
        }
    }

    void ReadDepthStencilAttachment(Target& target, const XML::Node* parent) {
        if (auto node = parent->FirstNode("depth")) {
            auto name = node->Text("name");
            if (DataTypeFromName(name) != Data::Type::Surface)
                throw Exception("Attachment name must have an surface type");
            target.depth_stencil_attachment_id = IdFromName(name);
        }
    }
};

struct BankBuild : public Bank {
    BankBuild(uint64 id, const String& name) : Bank(id, name) {
        ReadOnlyFile xml_file(AssetFilename(name));
        XML::Doc doc((char*)xml_file.Pointer());
        auto root = doc.FirstNode();

        const auto path = AssetPath() + File::ExtractPath(name);
        Array<ReadOnlyFile, SoundMaxCount> wav_files;
        Array<WAV, SoundMaxCount> wavs;
        size total_data_size = 0;
        ReadSounds(root, path, total_data_size, wav_files, wavs);

        WriteOnlyFile data_file(CacheDataFilename(name), total_data_size);
        size out_offset = 0;
        wav_files.ConstProcessIndex([&](const auto& wav_file, unsigned index) {
            memcpy((uint8*)data_file.Pointer() + out_offset, (uint8*)wav_file.Pointer() + wavs[index].Offset(), wavs[index].Length());
            out_offset += wavs[index].Length();
        });
    }

    void ReadSounds(const XML::Node* parent, const String& path, size& total_data_size, Array<ReadOnlyFile, SoundMaxCount>& wav_files, Array<WAV, SoundMaxCount>& wavs) {
        auto node = parent->FirstNode("sound");
        while (node) {
            const auto name = node->Text("name");
            const auto wav_filename = path + name + ".wav";
            wav_files.Add(wav_filename);
            wavs.Add((uint8*)wav_files.Back().Pointer(), wav_files.Back().Size());
            sounds.Add(Hash::Fnv32(name.Data()), (char*)&wavs.Back().wfxt, sizeof(WAV::WAVEFORMATEXTENSIBLE), total_data_size, wavs.Back().Length());
            total_data_size += wavs.Back().Length();
            node = node->NextSibling("sound");
        }
    }
};
static_assert(sizeof(WAV::WAVEFORMATEXTENSIBLE) <= Bank::SoundFormatMaxSize);

struct ScriptBuild : public Script {
    ScriptBuild(uint64 id, const String& name) : Script(id, name) {
        ReadOnlyFile xml_file(AssetFilename(name));
        XML::Doc doc((char*)xml_file.Pointer());
        auto root = doc.FirstNode();

        const auto optimization = root->Text("optimization");
        Compile(name.Data(), optimization.Data());
    }

    static void Compile(const LongString& name, const LongString& optimization);
};

struct MeshBuild : public Mesh {
    MeshBuild(uint64 id, const String& name) : Mesh(id, name) {
        ReadOnlyFile xml_file(AssetFilename(name));

        ReadOnlyFile ply_file(AssetPath() + name + ".ply");

        PLY ply((char*)ply_file.Pointer());

        vertex_count = ply.Count("vertex");
        index_count = ply.Count("face") * 3;

        auto* x = ply.First("x");
        auto* y = ply.First("y");
        auto* z = ply.First("z");
        auto* nx = ply.First("nx");
        auto* ny = ply.First("ny");
        auto* nz = ply.First("nz");
        auto* s = ply.First("s");
        auto* t = ply.First("t");
        auto* r = ply.First("red");
        auto* g = ply.First("green");
        auto* b = ply.First("blue");

        auto* index = ply.First("vertex_indices");

        const bool has_position = (x != nullptr) && (y != nullptr) && (z != nullptr);
        const bool has_normals = (nx != nullptr) && (ny != nullptr) && (nz != nullptr);
        const bool has_texcoords0 = (s != nullptr) && (t != nullptr);
        const bool has_colors0 = (r != nullptr) && (g != nullptr) && (b != nullptr);

        size vertices_offset = 0;
        size normals_offset = 0;
        size texcoords0_offset = 0;
        size colors0_offset = 0;

        const size vertex_size = ComputeVertexSize(has_position, has_normals, has_texcoords0, has_colors0, vertices_offset, normals_offset, texcoords0_offset, colors0_offset);
        const auto attribute_type = ComputeAttributeType(index_count);
        const size index_size = ComputeIndexSize(index_count);

        const size vertices_size = vertex_size * vertex_count;
        const size indices_size = index_size * index_count;

        WriteOnlyFile data_file(CacheDataFilename(name), vertices_size + indices_size);
        OutputVertices(vertex_size, has_position, has_normals, has_texcoords0, has_colors0, x, y, z, nx, ny, nz, s, t, r, g, b, (uint8*)data_file.Pointer());
        OutputIndices(attribute_type, index, (uint8*)data_file.Pointer() + vertices_size);
        FillAttributes(vertex_size, index_size, attribute_type, vertices_size, has_position, has_normals, has_texcoords0, has_colors0, vertices_offset, normals_offset, texcoords0_offset, colors0_offset);
    }

    size ComputeVertexSize(bool has_position, const bool has_normals, const bool has_texcoords0, bool has_colors0,
        size& vertices_offset, size& normals_offset, size& texcoords0_offset, size& colors0_offset) {
        size vertex_size = 0;
        vertices_offset = vertex_size;      vertex_size += has_position ? Math::AlignSize((size)sizeof(float) * 3, (size)4) : 0;
        normals_offset = vertex_size;       vertex_size += has_normals ? Math::AlignSize((size)sizeof(int8) * 3, (size)4) : 0;
        texcoords0_offset = vertex_size;    vertex_size += has_texcoords0 ? Math::AlignSize((size)sizeof(uint16) * 2, (size)4) : 0;
        colors0_offset = vertex_size;       vertex_size += has_colors0 ? Math::AlignSize((size)sizeof(uint8) * 4, (size)4) : 0;
        if (vertex_size > 256) throw Exception("Vertex size is too big (max 256 bytes)");
        return vertex_size;
    }

    static Attribute::Type ComputeAttributeType(unsigned index_count) { return (index_count > 65536) ? Attribute::Type::U32 : Attribute::Type::U16; }
    static size ComputeIndexSize(unsigned index_count) { return (index_count > 65536) ? sizeof(uint32) : sizeof(uint16); }

    void OutputVertices(size vertex_size, bool has_position, bool has_normals, bool has_texcoords0, bool has_colors0,
        PLY::Value*& x, PLY::Value*& y, PLY::Value*& z,
        PLY::Value*& nx, PLY::Value*& ny, PLY::Value*& nz,
        PLY::Value*& s, PLY::Value*& t,
        PLY::Value*& r, PLY::Value*& g, PLY::Value*& b,
        uint8* vertices) const {
        for (unsigned n = 0; n < vertex_count; ++n) {
            uint8* out_vertices = vertices + n * vertex_size;
            if (has_position) OutputVerticesPosition(x, y, z, out_vertices);
            if (has_normals) OutputVerticesNormal(nx, ny, nz, out_vertices);
            if (has_texcoords0) OutputVerticesTexCoords(s, t, out_vertices);
            if (has_colors0) OutputVerticesColor(r, g, b, out_vertices);
        }
    }

    void OutputIndices(Attribute::Type attribute_type, PLY::Value*& index, uint8* indices) {
        uint8* out_indices = indices;
        switch (attribute_type) {
        case Attribute::Type::U16: OutputIndices16(index, out_indices); break;
        case Attribute::Type::U32: OutputIndices32(index, out_indices); break;
        default: break;
        };
    }

    void OutputVerticesPosition(PLY::Value*& x, PLY::Value*& y, PLY::Value*& z, uint8*& out_vertices) const {
        ((float*)out_vertices)[0] = x->Float(); x = x->Next();
        ((float*)out_vertices)[1] = y->Float(); y = y->Next();
        ((float*)out_vertices)[2] = z->Float(); z = z->Next();
        out_vertices += Math::AlignSize((size)sizeof(float) * 3, (size)4);
    }

    void OutputVerticesNormal(PLY::Value*& nx, PLY::Value*& ny, PLY::Value*& nz, uint8*& out_vertices) const {
        ((int8*)out_vertices)[0] = (int8)(nx->Float() * 127.0); nx = nx->Next();
        ((int8*)out_vertices)[1] = (int8)(ny->Float() * 127.0); ny = ny->Next();
        ((int8*)out_vertices)[2] = (int8)(nz->Float() * 127.0); nz = nz->Next();
        out_vertices += Math::AlignSize((size)sizeof(int8) * 3, (size)4);
    }

    void OutputVerticesTexCoords(PLY::Value*& s, PLY::Value*& t, uint8*& out_vertices) const {
        ((uint16*)out_vertices)[0] = Math::HalfCompress(s->Float()); s = s->Next();
        ((uint16*)out_vertices)[1] = Math::HalfCompress(t->Float()); t = t->Next();
        out_vertices += Math::AlignSize((size)sizeof(uint16) * 2, (size)4);
    }

    void OutputVerticesColor(PLY::Value*& r, PLY::Value*& g, PLY::Value*& b, uint8*& out_vertices) const {
        ((uint8*)out_vertices)[0] = (uint8)r->Int(); r = r->Next();
        ((uint8*)out_vertices)[1] = (uint8)g->Int(); g = g->Next();
        ((uint8*)out_vertices)[2] = (uint8)b->Int(); b = b->Next();
        ((uint8*)out_vertices)[3] = (uint8)255;
        out_vertices += Math::AlignSize((size)sizeof(uint8) * 4, (size)4);
    }

    void OutputIndices16(PLY::Value*& index, uint8*& out_indices) const {
        for (unsigned i = 0; i < index_count; ++i) {
            ((uint16*)out_indices)[0] = index->Int(); index = index->Next();
            out_indices += sizeof(uint16);
        }
    }

    void OutputIndices32(PLY::Value*& index, uint8*& out_indices) const {
        for (unsigned i = 0; i < index_count; ++i) {
            ((uint32*)out_indices)[0] = index->Int(); index = index->Next();
            out_indices += sizeof(uint32);
        }
    }

    void FillAttributes(size vertex_size, size index_size, Attribute::Type attribute_type, size vertices_size,
        bool has_position, bool has_normals, bool has_texcoords0, bool has_colors0,
        size vertices_offset, size normals_offset, size texcoords0_offset, size colors0_offset) {
        vertices.Fill(Attribute::Type::F32, (uint8)vertex_size, (uint8)vertices_offset, 0);
        normals.Fill(Attribute::Type::None, 0, 0, 0);
        if (has_normals) normals.Fill(Attribute::Type::S8, (uint8)vertex_size, (uint8)normals_offset, 0);
        if (has_texcoords0) uv_sets.Add().Fill(Attribute::Type::F16, (uint8)vertex_size, (uint8)texcoords0_offset, 0);
        if (has_colors0) color_sets.Add().Fill(Attribute::Type::U8, (uint8)vertex_size, (uint8)colors0_offset, 0);
        indices.Fill(attribute_type, (uint8)index_size, 0, (uint32)vertices_size);
    }
};

struct ShaderBuild : public Shader {
    ShaderBuild(uint64 id, const String& name) : Shader(id, name) {
        ReadOnlyFile xml_file(AssetFilename(name));

        XML::Doc doc((char*)xml_file.Pointer());
        auto root = doc.FirstNode();

        ReadInputs(root);
        ReadDescriptors(root);
        ReadSamplers(root);
        ReadTechniques(root);

        ReadOnlyFile hlsl_file(AssetPath() + name + ".hlsl");

        Array<Alloc, TechniqueMaxCount> bytecodes;
        size total_bytecode_size = 0;
        techniques.Process([&](auto& technique) {
            CompileTechnique(technique, (char*)hlsl_file.Pointer(), hlsl_file.Size(), hlsl_file.FileName(), bytecodes, total_bytecode_size);
        });

        WriteOnlyFile data_file(CacheDataFilename(name), total_bytecode_size);
        size out_offset = 0;
        bytecodes.ConstProcessIndex([&](const auto& bytecode, unsigned index) {
            memcpy((uint8*)data_file.Pointer() + out_offset, (uint8*)bytecode.Pointer(), bytecode.Size());
            out_offset += bytecode.Size();
        });
    }

    void CompileTechnique(Technique& technique, const char* source, size source_size, const String& filename, Array<Alloc, TechniqueMaxCount>& bytecodes, size& total_bytecode_size);
    static Alloc Compile(const char* source, size source_size, const String& filename, const String& entry_point, const char* target, Binary& binary, size& total_bytecode_size);

    void ReadTechniques(const XML::Node* parent) {
        auto node = parent->FirstNode("technique");
        while (node) {
            auto& technique = techniques.Add();
            ReadTechnique(node, technique);
            node = node->NextSibling("technique");
        }
        if (techniques.UsedCount() == 0) throw Exception("Should have at least 1 technique");
    }

    void ReadInputs(const XML::Node* parent) {
        auto node = parent->FirstNode("input");
        while (node) {
            auto& input = inputs.Add();
            ReadInput(node, input);
            node = node->NextSibling("input");
        }
    }

    void ReadInput(const XML::Node* parent, Shader::Input& input) {
        ReadSemantic(parent, input);
        input.index = parent->Int("index", 0);
        ReadFormat(parent, input);
    }

    void ReadSemantic(const XML::Node* parent, Shader::Input& input) {
        auto name = parent->Text("semantic");
        if (name == "position") input.semantic = Input::Semantic::Position;
        else if (name == "normal") input.semantic = Input::Semantic::Normal;
        else if (name == "texcoord") input.semantic = Input::Semantic::Texcoord;
        else if (name == "color") input.semantic = Input::Semantic::Color;
        if (input.semantic == Input::Semantic::None) throw Exception("Invalid 'semantic' XML attribute");
    }

    void ReadFormat(const XML::Node* parent, Shader::Input& input) {
        auto name = parent->Text("format");
        if (name == "char4") input.format = Input::Format::char4;
        else if (name == "uchar4") input.format = Input::Format::uchar4;
        else if (name == "half2") input.format = Input::Format::half2;
        else if (name == "float3") input.format = Input::Format::float3;
        if (input.format == Input::Format::None) throw Exception("Invalid 'format' XML attribute");
    }

    void ReadDescriptors(const XML::Node* parent) {
        auto node = parent->FirstNode("descriptor");
        while (node) {
            auto& descriptor = descriptors.Add();
            ReadDescriptor(node, descriptor);
            node = node->NextSibling("descriptor");
        }
    }

    void ReadDescriptor(const XML::Node* parent, Descriptor& descriptor) {
        ReadType(parent, descriptor);
    }

    void ReadType(const XML::Node* parent, Descriptor& descriptor) {
        auto name = parent->Text("type");
        if (name == "texture") descriptor.type = Descriptor::Type::Texture;
        else if (name == "buffer") descriptor.type = Descriptor::Type::Buffer;
        if (descriptor.type == Descriptor::Type::None) throw Exception("Invalid 'type' XML attribute");
    }

    void ReadSamplers(const XML::Node* parent) {
        auto node = parent->FirstNode("sampler");
        while (node) {
            auto& sampler = samplers.Add();
            ReadSampler(node, sampler);
            node = node->NextSibling("sampler");
        }
    }

    void ReadSampler(const XML::Node* parent, Sampler& sampler) {
        ReadFilter(parent, sampler);
        ReadAddressMode(parent, sampler);
        ReadComparison(parent, sampler);
    }

    void ReadFilter(const XML::Node* parent, Sampler& sampler) {
        auto name = parent->Text("filter");
        if (name == "point") sampler.filter = Sampler::Filter::MinMagMipPoint;
        else if (name == "linear") sampler.filter = Sampler::Filter::MinMagMipLinear;
        else if (name == "anisotropic") sampler.filter = Sampler::Filter::Anisotropic;
        else if (name == "comparison_linear") sampler.filter = Sampler::Filter::ComparisonMinMagMipLinear;
        else if (name == "comparison_anisotropic") sampler.filter = Sampler::Filter::ComparisonAnisotropic;
        if (sampler.filter == Sampler::Filter::None) throw Exception("Invalid 'filter' XML attribute");
    }

    void ReadAddressMode(const XML::Node* parent, Sampler& sampler) {
        auto name = parent->Text("address");
        if (name == "wrap") sampler.address_mode = Sampler::AddressMode::Wrap;
        else if (name == "mirror") sampler.address_mode = Sampler::AddressMode::Mirror;
        else if (name == "clamp") sampler.address_mode = Sampler::AddressMode::Clamp;
        else if (name == "border") sampler.address_mode = Sampler::AddressMode::Border;
        else if (name == "mirror_once") sampler.address_mode = Sampler::AddressMode::MirrorOnce;
        if (sampler.address_mode == Sampler::AddressMode::None) throw Exception("Invalid 'address' XML attribute");
    }

    void ReadComparison(const XML::Node* parent, Sampler& sampler) {
        auto name = parent->Text("comparison");
        if (name == "never") sampler.comparison = Sampler::Comparison::Never;
        else if (name == "always") sampler.comparison = Sampler::Comparison::Always;
        else if (name == "equal") sampler.comparison = Sampler::Comparison::Equal;
        else if (name == "notequal") sampler.comparison = Sampler::Comparison::NotEqual;
        else if (name == "greater") sampler.comparison = Sampler::Comparison::Greater;
        else if (name == "greaterequal") sampler.comparison = Sampler::Comparison::GreaterEqual;
        else if (name == "less") sampler.comparison = Sampler::Comparison::Less;
        else if (name == "lessequal") sampler.comparison = Sampler::Comparison::LessEqual;
        if (sampler.comparison == Sampler::Comparison::None) throw Exception("Invalid 'address' XML attribute");
    }

    void ReadTechnique(const XML::Node* parent, Technique& technique) {
        technique.technique_id = parent->Hash32("name");
        ReadStateFlags(parent, technique);
        for (unsigned i = 0; i < Camera::ColorAttachmentMaxCount; ++i)
            ReadColorAttachment(parent, technique, i);
        ReadDepthAttachment(parent, technique);
        technique.depth_bias = parent->Float("depth_bias", 0.0f);
        technique.slope_scale_depth_bias = parent->Float("slope_scale_depth_bias", 0.0f);
        technique.vertex_function_name = parent->Text("vertex");
        technique.pixel_function_name = parent->Text("fragment");
        if (technique.vertex_function_name.Size() == 0) throw Exception("Missing VS");
        if (technique.pixel_function_name.Size() == 0) throw Exception("Missing PS");
    }

    void ReadStateFlags(const XML::Node* parent, Technique& technique) {
        technique.state_flags = State::None;
        technique.state_flags |= parent->Bool("depth_test", false) ? State::DepthTest : State::None;
        technique.state_flags |= parent->Bool("depth_write", false) ? State::DepthWrite : State::None;
        auto alpha = parent->Text("alpha");
        if (alpha != "") {
            if (alpha == "pre") { technique.state_flags |= State::Alpha; technique.state_flags |= State::AlphaPreMultiplied; }
            if (alpha == "post") { technique.state_flags |= State::Alpha; technique.state_flags |= State::AlphaPostMultiplied; }
            if (alpha == "add") { technique.state_flags |= State::Alpha; technique.state_flags |= State::AlphaAdditive; }
            if (alpha == "mul") { technique.state_flags |= State::Alpha; technique.state_flags |= State::AlphaMultiplicative; }
        }
        auto cull = parent->Text("cull");
        if (cull != "") {
            if (cull == "front") { technique.state_flags |= State::FrontFaceCull; }
            if (cull == "back") { technique.state_flags |= State::BackFaceCull; }
        }
    }

    void ReadDepthAttachment(const XML::Node* parent, Technique& technique) {
        auto format = parent->Text("depth");
        if (format == "")
            return;
        PixelFormat pixel_format = PixelFormat::None;
        if (format == "D16") pixel_format = PixelFormat::D16;
        else if (format == "D32F") pixel_format = PixelFormat::D32F;
        if (pixel_format == PixelFormat::None) throw Exception("Invalid 'depth' XML attribute");
        technique.depth_attachment_pixel_format = pixel_format;
    }

    void ReadColorAttachment(const XML::Node* parent, Technique& technique, unsigned i) {
        char buf[16];
        if (snprintf(buf, 16, "color%u", i) < 0) throw Exception();
        auto format = parent->Text(buf);
        if (format == "")
            return;
        PixelFormat pixel_format = PixelFormat::None;
        if (format == "R16F") pixel_format = PixelFormat::R16F;
        else if (format == "R32F") pixel_format = PixelFormat::R32F;
        else if (format == "RGB11F") pixel_format = PixelFormat::RGB11F;
        else if (format == "RGBA8") pixel_format = PixelFormat::RGBA8;
        else if (format == "BGRA8") pixel_format = PixelFormat::BGRA8;
        else if (format == "RGBA10") pixel_format = PixelFormat::RGBA10;
        else if (format == "RGBA16F") pixel_format = PixelFormat::RGBA16F;
        if (pixel_format == PixelFormat::None) throw Exception("Invalid 'color' XML attribute");
        technique.color_attachment_pixel_formats.Add(pixel_format);
    }
};

struct SurfaceBuild : public Surface {
    SurfaceBuild(uint64 id, const String& name) : Surface(id, name) {
        ReadOnlyFile xml_file(AssetFilename(name));
        XML::Doc doc((char*)xml_file.Pointer());
        auto root = doc.FirstNode();

        is_texture = root->Bool("texture", false);
        if (is_texture) {
            ReadTexture(root);
        } else {
            ReadAttachment(root);
        }
    }

    void ReadTexture(const XML::Node* parent) {
        const bool mips = parent->Bool("mips", true);
        Convert(mips);
    }

    void ReadAttachment(const XML::Node* parent) {
        width = 1;
        height = 1;
        is_default = parent->Bool("default", false);
        resize = Math::Clamp(parent->Float("resize", 0.f), 0.f, 4.f);
        parent->Vec2(&sizes.x, "size", 0.f);
        pixel_format = ReadPixelFormat(parent);
        auto node = parent->FirstNode("clear");
        do_clear = node != nullptr;
        clear_color.x = node ? node->Float("R", 0.f) : 0.f;
        clear_color.y = node ? node->Float("G", 0.f) : 0.f;
        clear_color.z = node ? node->Float("B", 0.f) : 0.f;
        clear_color.w = node ? node->Float("A", 0.f) : 0.f;
        is_depth_stencil = (pixel_format == PixelFormat::D16) || (pixel_format == PixelFormat::D32F);
    }

    PixelFormat ReadPixelFormat(const XML::Node* parent) {
        auto format = parent->Text("format");
        if (format == "D16") pixel_format = PixelFormat::D16;
        else if (format == "D32F") return PixelFormat::D32F;
        else if (format == "R16F") return PixelFormat::R16F;
        else if (format == "R32F") return PixelFormat::R32F;
        else if (format == "RGB11F") return PixelFormat::RGB11F;
        else if (format == "BGRA8") return PixelFormat::BGRA8;
        else if (format == "RGBA8") return PixelFormat::RGBA8;
        else if (format == "RGBA10") return PixelFormat::RGBA10;
        else if (format == "RGBA16F") return PixelFormat::RGBA16F;
        throw Exception("Invalid 'format' XML attribute");
    }

    void Convert(bool mips);
};

class Builder : public NoCopy {
    bool IsOutOfDate(const String& name) {
        bool res = false;
        ReadQueryFile header_file(CacheHeaderFilename(name));
        const auto sub_path = File::ExtractPath(name);
        const auto path = File::ExtractPath(AssetPath() + name);
        Directory::ProcessFiles(path.Data(), [&](const String& filename) {
            const auto full_name = sub_path + filename;
            if (!full_name.StartsWith(name))
                return;
            ReadQueryFile asset_file(AssetPath() + name + File::GetExtension(filename));
            res = res || asset_file.IsNewer(header_file);
        });
        return res;
    }

    template<typename T, typename B> void Create(uint64 id, const String& name) {
        WriteOnlyFile header_file(CacheHeaderFilename(name), sizeof(T));
        new(header_file.Pointer()) B(id, name);
    }

    void Build(const uint64 id, const String& name) {
        switch (Data::DataTypeFromId(id)) {
        case Data::Type::Camera: Create<Camera, CameraBuild>(id, name); break;
        case Data::Type::Cell: Create<Cell, CellBuild>(id, name); break;
        case Data::Type::Dictionary: Create<Dictionary, DictionaryBuild>(id, name); break;
        case Data::Type::Flags: Create<Flags, FlagsBuild>(id, name); break;
        case Data::Type::Follow: Create<Follow, FollowBuild>(id, name); break;
        case Data::Type::Mesh: Create<Mesh, MeshBuild>(id, name); break;
        case Data::Type::Script: Create<Script, ScriptBuild>(id, name); break;
        case Data::Type::Surface: Create<Surface, SurfaceBuild>(id, name); break;
        case Data::Type::Shader: Create<Shader, ShaderBuild>(id, name); break;
        case Data::Type::Uniforms: Create<Uniforms, UniformsBuild>(id, name); break;
        case Data::Type::Bank: Create<Bank, BankBuild>(id, name); break;
        case Data::Type::Source: Create<Source, SourceBuild>(id, name); break;
        default: break;
        };
    }

    void TryBuild(const String& name) {
        try {
            if (!File::Exist(CacheHeaderFilename(name)) || IsOutOfDate(name)) {
                Timer timer;
                const auto start = timer.Now();
                const uint64 id = Data::IdFromName(name);
                if (Data::DataTypeFromId(id) == Data::Type::Invalid) throw Exception("Invalid data type");
                File::CreatePath(CacheHeaderFilename(name));
                Build(id, name);
                const auto duration = (timer.Now() - start) * 0.000001;
                Log::Put("Build %s in %llf seconds\n", name.Data(), duration);
            }
        }
        catch (const Exception& exception) {
            Log::Put("Build %s FAIL\n %s\n", name.Data(), exception.Text());
            if (File::Exist(CacheHeaderFilename(name)) && !File::Exist(CacheDataFilename(name))) {
                File::Delete(CacheHeaderFilename(name));
            }
        }
    }

    void BuildFolder(const String& path) {
        Directory::ProcessFiles(path.Data(), [&](const String& filename) {
            if (!filename.EndsWith(".xml"))
                return;
            auto name = path.SubString(AssetPath().Size()) + filename.SubString(0, filename.Size() - 4);
            TryBuild(name);
        });
    }

public:
    Builder() {
        Log::Put("Build\n");
        Timer timer;
        const auto start = timer.Now();
        BuildFolder(AssetPath());
        Directory::ProcessFolders(AssetPath().Data(), [&](const String& path) {
            BuildFolder(path);
        });
        const auto duration = (timer.Now() - start) * 0.000001;
        Log::Put("Build in %llf seconds\n", duration);
    }
};

class Packager : public NoCopy {
    static const unsigned ResourceMaxCount = 256;

    struct Resource {
        uint64 data_id = 0;
        Alloc alloc;

        Resource() {}
        Resource(Resource& other) : data_id(other.data_id), alloc(other.alloc) { other.data_id = 0; }
        Resource(uint64 data_id, const String& path) : data_id(data_id) {
            ReadOnlyFile file(path);
            new(&alloc) Alloc(file.Pointer(), file.Size());
        }

        Resource& operator=(Resource& other) { data_id = other.data_id; alloc = other.alloc; other.data_id = 0; return *this; }

        bool operator>(const Resource& other) const { return data_id > other.data_id; }
    };

    void GatherFolder(const String& path, Array<Resource, ResourceMaxCount>& headers, Array<Resource, ResourceMaxCount>& datas) {
        Directory::ProcessFiles(path.Data(), [&](const String& filename) {
            auto name = path.SubString(CachePath().Size()) + filename;
            if (filename.EndsWith(".data"))
                datas.Add(Data::IdFromName(name.SubString(0, name.Size() - 5)), path + filename);
            else if (filename.EndsWith(".header"))
                headers.Add(Data::IdFromName(name.SubString(0, name.Size() - 7)), path + filename);
        });
    }

    void Gather(Array<Resource, ResourceMaxCount>& headers, Array<Resource, ResourceMaxCount>& datas) {
        GatherFolder(CachePath(), headers, datas);
        Directory::ProcessFolders(CachePath().Data(), [&](const String& path) {
            GatherFolder(path, headers, datas);
        });
        headers.Sort();
        datas.Sort();
    }

    static size SizeResources(const Array<Resource, ResourceMaxCount>& resources) {
        size size = 0;
        resources.ConstProcess([&](auto& resource) {
            size += resource.alloc.Size() + Data::DynamicSize;
        });
        return size;
    }

    uint8* WriteCount(uint8* out, unsigned entries_count) {
        *(uint32*)out = entries_count;
        out += sizeof(uint32);
        return out;
    }

    uint8* WriteTable(uint8* out, const Array<Resource, ResourceMaxCount>& resources, size start) {
        size offset = start;
        resources.ConstProcessIndex([&](auto& resource, unsigned index) {
            const size size = resource.alloc.Size();
            ((Bundle::Resource*)out)[index] = Bundle::Resource(resource.data_id, offset, size);
            offset += size + Data::DynamicSize;
        });
        out += resources.UsedCount() * sizeof(Bundle::Resource);
        return out;
    }

    uint8* WriteResources(uint8* out, const Array<Resource, ResourceMaxCount>& resources) {
        resources.ConstProcess([&](auto& resource) {
            const size size = resource.alloc.Size();
            memcpy(out, resource.alloc.Pointer(), size);
            out += size;
            memset(out, 0, Data::DynamicSize);
            out += Data::DynamicSize;
        });
        return out;
    }

    void Write(const Array<Resource, ResourceMaxCount>& headers, const Array<Resource, ResourceMaxCount>& datas) {
        const unsigned header_count = headers.UsedCount();
        const unsigned data_count = datas.UsedCount();
        const size headers_size = SizeResources(headers);
        const size datas_size = SizeResources(datas);
        const size resources_size = header_count * sizeof(Bundle::Resource);
        const size entries_size = data_count * sizeof(Bundle::Resource);
        const size metadata_size = sizeof(uint32) * 2 + resources_size + entries_size;
        const size total_size = metadata_size + headers_size + datas_size;
        WriteOnlyFile bundle_file(Bundle::Name(), total_size);
        auto* out = (uint8*)bundle_file.Pointer();
        out = WriteCount(out, header_count);
        out = WriteCount(out, data_count);
        out = WriteTable(out, headers, metadata_size);
        out = WriteTable(out, datas, metadata_size + headers_size);
        out = WriteResources(out, headers);
        out = WriteResources(out, datas);
    }

public:
    Packager() {
        Log::Put("Package\n");
        Timer timer;
        const auto start = timer.Now();
        Array<Resource, ResourceMaxCount> headers;
        Array<Resource, ResourceMaxCount> datas;
        Gather(headers, datas);
        Write(headers, datas);
        const auto duration = (timer.Now() - start) * 0.000001;
        Log::Put("Package in %llf seconds\n", duration);
    }
};

