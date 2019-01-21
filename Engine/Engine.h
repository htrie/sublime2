
class Stack : public NoCopy {
    static const size StackSize = 256 * 1024;

    FixedArray<Buffer, Context::BufferCount> buffers;
    unsigned index = 0;
    size offset = 0;

public:
    Stack() {}
    Stack(Context* context) {
        buffers.Process([&](auto& buffer) {
            new(&buffer) Buffer(*context, StackSize);
        });
    }

    void Swap() {
        index = (index + 1) % Context::BufferCount;
        offset = 0;
    }

    void Allocate(size size, uint8*& cpu, uint64& gpu) {
        const auto aligned_size = Buffer::AlignSize(size);
        DEBUG_ONLY(if (offset + aligned_size >= StackSize) throw Exception("Out-of-bounds");)
        cpu = buffers[index].CPU() + offset;
        gpu = buffers[index].GPU() + offset;
        offset += aligned_size;
    }
};

struct ClusterDynamic : public NoCopy {
    Cluster* cluster = nullptr; // TODO: Remove.

    void Load(Cluster* cluster, Context& context) { this->cluster = cluster; }
};

struct ScriptClusterDynamic : public ScriptCluster, ClusterDynamic {
    char padding[Cluster::DynamicSize - sizeof(ClusterDynamic)];
};

struct FollowClusterDynamic : public FollowCluster, ClusterDynamic {
    char padding[Cluster::DynamicSize - sizeof(ClusterDynamic)];
};

struct RenderClusterDynamic : public RenderCluster, ClusterDynamic {
    char padding[Cluster::DynamicSize - sizeof(ClusterDynamic)];
};

struct SourceClusterDynamic : public SourceCluster, ClusterDynamic {
    FixedArray<Voice, Cluster::BatchMaxCount> voices;
    char padding[Cluster::DynamicSize - sizeof(ClusterDynamic) - sizeof(FixedArray<Voice, Cluster::BatchMaxCount>)];

    void Load(Cluster* cluster, Context& context) {
        ClusterDynamic::Load(cluster, context);
        voices.Process([&](auto& voice) {
            new(&voice) Voice(); // Init virtual table.
        });
    }
};

struct CameraClusterDynamic : public CameraCluster, ClusterDynamic {
    CommandList command_list;
    uint64 camera_uniforms_gpu = 0;
    Camera::Uniforms* camera_uniforms_cpu = nullptr;
    Timings timings;
    char padding[Cluster::DynamicSize - sizeof(ClusterDynamic) - sizeof(CommandList) - sizeof(uint64) - sizeof(Camera::Uniforms*) - sizeof(Timings)];

    void Load(Cluster* cluster, Context& context) {
        ClusterDynamic::Load(cluster, context);
        new(&command_list) CommandList(context);
        new(&timings) Timings(context);
    }
};

static_assert(sizeof(ScriptClusterDynamic) == (sizeof(ScriptCluster) + Cluster::DynamicSize));
static_assert(sizeof(FollowClusterDynamic) == (sizeof(FollowCluster) + Cluster::DynamicSize));
static_assert(sizeof(RenderClusterDynamic) == (sizeof(RenderCluster) + Cluster::DynamicSize));
static_assert(sizeof(SourceClusterDynamic) == (sizeof(SourceCluster) + Cluster::DynamicSize));
static_assert(sizeof(CameraClusterDynamic) == (sizeof(CameraCluster) + Cluster::DynamicSize));

class CellDynamic : public Cell {
    Cluster* FindCluster(uint64 cluster_id) {
        return clusters.BinaryFind((Cluster&)Named(cluster_id));
    }

    template <typename C> void LoadTable(ProxyArray<C>& clusters_dynamic, const Table& table, const uint8* data, Context& context) {
        new(&clusters_dynamic) ProxyArray<C>((C*)(data + table.offset), table.count);
        clusters_dynamic.Process([&](auto& cluster_dynamic) {
            auto* cluster = FindCluster(cluster_dynamic.cluster_id); // TODO: Avoid find.
            cluster_dynamic.Load(cluster, context);
        });
    }

public:
    ProxyArray<Cluster> clusters;
    ProxyArray<ScriptClusterDynamic> script_clusters;
    ProxyArray<FollowClusterDynamic> follow_clusters;
    ProxyArray<SourceClusterDynamic> source_clusters;
    ProxyArray<CameraClusterDynamic> camera_clusters;
    ProxyArray<RenderClusterDynamic> render_clusters;

    void Load(const Bundle& bundle, Context& context) {
        const auto data = bundle.FindData(Id());
        new(&clusters) ProxyArray<Cluster>((Cluster*)(data.Mem() + cluster_offset), cluster_count);
        LoadTable(script_clusters, cluster_tables[Table::Script], data.Mem(), context);
        LoadTable(follow_clusters, cluster_tables[Table::Follow], data.Mem(), context);
        LoadTable(source_clusters, cluster_tables[Table::Source], data.Mem(), context);
        LoadTable(camera_clusters, cluster_tables[Table::Camera], data.Mem(), context);
        LoadTable(render_clusters, cluster_tables[Table::Render], data.Mem(), context);
    }
};

class BundleDynamic : public Bundle {
    CellDynamic* cell; // TODO: Put in Bundle.

public:
    BundleDynamic() {
        const auto cell_id = Data::IdFromName("Cells/All.cell");
        cell = Find<CellDynamic>(cell_id);
    }

    template<typename F> void ProcessCameraClusters(F func) {
        cell->camera_clusters.Process([&](auto& camera_cluster) {
            func(camera_cluster);
        });
    }

    template<typename F> void ProcessRenderClusters(F func) {
        cell->render_clusters.Process([&](auto& render_cluster) {
            func(render_cluster);
        });
    }

    template<typename F> void ProcessFollowClusters(F func) {
        cell->follow_clusters.Process([&](auto& follow_cluster) {
            func(follow_cluster);
        });
    }

    template<typename F> void ProcessScriptClusters(F func) {
        cell->script_clusters.Process([&](auto& script_cluster) {
            func(script_cluster);
        });
    }

    template<typename F> void ProcessSourceClusters(F func) {
        cell->source_clusters.Process([&](auto& source_cluster) {
            func(source_cluster);
        });
    }

    Cluster* FindCluster(uint64 cluster_id) {
        return cell->clusters.BinaryFind((Cluster&)Named(cluster_id));
    }

    FollowClusterDynamic* FindFollowCluster(uint64 cluster_id) {
        return cell->follow_clusters.BinaryFind((FollowClusterDynamic&)ClusterId(cluster_id));
    }

    CameraClusterDynamic* FindCameraCluster(uint64 cluster_id) {
        return cell->camera_clusters.BinaryFind((CameraClusterDynamic&)ClusterId(cluster_id));
    }

    RenderClusterDynamic* FindRenderCluster(uint64 cluster_id) {
        return cell->render_clusters.BinaryFind((RenderClusterDynamic&)ClusterId(cluster_id));
    }

    SourceClusterDynamic* FindSourceCluster(uint64 cluster_id) {
        return cell->source_clusters.BinaryFind((SourceClusterDynamic&)ClusterId(cluster_id));
    }
};

class Common {
protected:
    Profile profile;
    Timer timer;
    BundleDynamic bundle;

    Common() {}
};

class Audio : public Common {
    Surround surround;

    void LoadAll() {
        bundle.Process([&](auto& data) {
            switch (Data::DataTypeFromId(data.Id())) {
            case Data::Type::Bank: ((BankDynamic&)data).Load(bundle, surround); break;
            default: break;
            }
        });
    }

    void UnloadAll() {
        bundle.Process([&](auto& data) {
            switch (Data::DataTypeFromId(data.Id())) {
            case Data::Type::Bank: ((BankDynamic&)data).~BankDynamic(); break;
            default: break;
            }
        });
    }

    void GarbageCollect() {
        bundle.ProcessSourceClusters([&](auto& source_cluster) {
            source_cluster.voices.Process([&](auto& voice) {
                voice.GarbageCollect();
            });
        });
    }

protected:
    Audio() {
        LoadAll();
    }

    ~Audio() {
        DEBUG_ONLY(surround.Stop();)
        DEBUG_ONLY(UnloadAll();)
    }

    void Update() {
        DEBUG_ONLY(profile.BeginCPU(timer.Now(), Color::Yellow);)
        GarbageCollect();
        DEBUG_ONLY(profile.EndCPU(timer.Now());)
    }

    bool Play(const Id& id, uint32 sound_id, float volume) {
        if (auto* source_cluster = bundle.FindSourceCluster(id.cluster_id))
            if (auto* bank = bundle.Find<BankDynamic>(source_cluster->bank_id)) {
                unsigned index = 0;
                if (const auto* batch = source_cluster->cluster->ConstFindIndex(id.batch_id, index)) {
                    if (auto* source = bundle.Find<Source>(source_cluster->source_ids[index]))
                        return source_cluster->voices[index].Play(surround, *source, *bank, sound_id, volume);
                }
            }
        return false;
    }
};

class Render : public Audio {
    Context context;
    Stack stack;
    uint64 gpu_frame_duration = 0;
    DEBUG_ONLY(DebugDraw debug_draw;)
    DEBUG_ONLY(DebugShapes debug_shapes;)
    DEBUG_ONLY(DebugProfile debug_profile;)
    DEBUG_ONLY(bool draw_bounds = false;)
    DEBUG_ONLY(bool draw_extents = false;)

    void LoadAll() {
        CommandList upload_command_list(context);
        upload_command_list.Reset(context);
        bundle.Process([&](auto& data) {
            switch (Data::DataTypeFromId(data.Id())) {
            case Data::Type::Cell: ((CellDynamic&)data).Load(bundle, context); break;
            case Data::Type::Mesh: ((MeshDynamic&)data).Load(bundle, context, upload_command_list); break;
            case Data::Type::Shader: ((ShaderDynamic&)data).Load(bundle, context); break;
            case Data::Type::Surface: ((SurfaceDynamic&)data).Load(bundle, context, upload_command_list); ((SurfaceDynamic&)data).Create(context); break;
            default: break;
            }
        });
        upload_command_list.Close(context);
        Array<CommandList*, 16> command_lists;
        command_lists.Add(&upload_command_list);
        CommandList::Execute(context, command_lists);
        context.Stop();
    }

    void UnloadAll() {
        bundle.Process([&](auto& data) {
            switch (Data::DataTypeFromId(data.Id())) {
            case Data::Type::Cell: ((CellDynamic&)data).~CellDynamic(); break;
            case Data::Type::Mesh: ((MeshDynamic*)&data)->~MeshDynamic(); break;
            case Data::Type::Shader: ((ShaderDynamic*)&data)->~ShaderDynamic(); break;
            case Data::Type::Surface: ((SurfaceDynamic*)&data)->Destroy(context); ((SurfaceDynamic&)data).~SurfaceDynamic(); break;
            default: break;
            }
        });
    }

    void Swap() {
        context.Swap();
        bundle.ProcessCameraClusters([&](auto& camera_cluster) {
            camera_cluster.timings.Swap(context, timer.Now());
        });
        stack.Swap();
        DEBUG_ONLY(debug_draw.Swap();)
        DEBUG_ONLY(debug_shapes.Swap();)
        DEBUG_ONLY(debug_profile.Swap(timer.Now());)
    }

    void UpdateCameras() {
        bundle.ProcessCameraClusters([&](auto& camera_cluster) {
            if (auto* camera = bundle.Find<Camera>(camera_cluster.camera_id)) {
#if !defined(__APPLE__) // TODO: Remove.
                stack.Allocate(sizeof(Camera::Uniforms), (uint8*&)camera_cluster.camera_uniforms_cpu, camera_cluster.camera_uniforms_gpu);
#endif
                if (camera_cluster.camera_uniforms_cpu)
                    camera->Update(*camera_cluster.camera_uniforms_cpu, camera_cluster.cluster->Batches()[0].Instances()[0].position, camera_cluster.cluster->Batches()[0].Instances()[0].rotation, context.WindowWidth(), context.WindowHeight());
            }
        });
    }

    void ProcessCameras() {
        bundle.ProcessCameraClusters([&](auto& camera_cluster) {
            if (auto* camera = bundle.Find<Camera>(camera_cluster.camera_id)) {
                camera_cluster.command_list.Reset(context);
                ProcessTargets(camera_cluster, *camera);
                camera_cluster.timings.Query(camera_cluster.command_list);
                camera_cluster.command_list.Close(context);
                camera_cluster.timings.GatherBegin();
                camera->Targets().ConstProcess([&](auto& target) {
                    target.passes.ConstProcess([&](auto& pass) {
                        camera_cluster.timings.GatherOne(profile, (Color)pass.profile);
                    });
                });
                camera_cluster.timings.GatherEnd();
            }
        });
    }

    void ProcessTargets(CameraClusterDynamic& camera_cluster, const Camera& camera) {
        camera.Targets().ConstProcess([&](auto& target) {
            auto attachments = GatherAttachments(camera_cluster, target);
            ProcessPasses(camera_cluster, target, attachments);
            if (target.last)
                DrawDebugLast(camera_cluster);
        });
    }

    void ProcessPasses(CameraClusterDynamic& camera_cluster, const Camera::Target& target, const Attachments& attachments) {
        target.passes.ConstProcess([&](auto& pass) {
            camera_cluster.timings.Push(camera_cluster.command_list);
            if (pass.auto_shader_id) {
                DrawSingle(camera_cluster, pass, attachments);
            } else {
                bundle.ProcessRenderClusters([&](auto& render_cluster) {
                    auto* self_camera_cluster = render_cluster.has_camera ? bundle.FindCameraCluster(render_cluster.cluster->Id()) : nullptr;
                    DrawCluster(camera_cluster, render_cluster, self_camera_cluster, pass, attachments);
                });
            }
            camera_cluster.timings.Push(camera_cluster.command_list);
        });
    }

    void DrawSingle(CameraClusterDynamic& camera_cluster, const Camera::Pass& pass, const Attachments& attachments) {
        if (auto* shader = bundle.Find<ShaderDynamic>(pass.auto_shader_id)) {
            unsigned technique_index = shader->FindTechnique(pass.technique_id);
            if (technique_index != (unsigned)-1) {
                auto surfaces = GatherSurfaces(pass.auto_surface_ids);
                if (auto* mesh = bundle.Find<MeshDynamic>(pass.auto_mesh_id))
                    if (auto* uniforms = bundle.Find<Uniforms>(pass.auto_uniforms_id)) {
                        SetShaderAndSurfaces(camera_cluster, nullptr, attachments, *shader, surfaces, technique_index);
                        const auto gpu = FillOne(camera_cluster);
                        SetMeshAndDraw(camera_cluster, *mesh, *uniforms, gpu, 1);
                    }
            }
        }
    }

    void DrawCluster(CameraClusterDynamic& camera_cluster, RenderClusterDynamic& render_cluster, CameraClusterDynamic* self_camera_cluster, const Camera::Pass& pass, const Attachments& attachments) {
        if (render_cluster.cluster->Bounds().Intersect(camera_cluster.cluster->Bounds())) {
            if (auto* flags = bundle.Find<Flags>(render_cluster.flags_id))
                if (flags->Check(pass.include_flags, pass.exclude_flags))
                    if (auto* shader = bundle.Find<ShaderDynamic>(render_cluster.shader_id)) {
                        unsigned technique_index = shader->FindTechnique(pass.technique_id);
                        if (technique_index != (unsigned)-1) {
                            auto surfaces = GatherSurfaces(render_cluster.surface_ids);
                            SetShaderAndSurfaces(camera_cluster, self_camera_cluster, attachments, *shader, surfaces, technique_index);
                            render_cluster.mesh_ids.ProcessIndex([&](auto& mesh_id, unsigned index) {
                                if (auto* mesh = bundle.Find<MeshDynamic>(mesh_id))
                                    if (auto* uniforms = bundle.Find<Uniforms>(render_cluster.uniforms_ids[index])) {
                                        const auto& batch = render_cluster.cluster->Batches()[index];
                                        const auto gpu = FillBatch(camera_cluster, batch);
                                        SetMeshAndDraw(camera_cluster, *mesh, *uniforms, gpu, batch.Instances().UsedCount());
                                    }
                            });
                        }
                    }
        }
    }

    void DrawDebugLast(CameraClusterDynamic& camera_cluster) {
        DEBUG_ONLY(debug_draw.Reset(camera_cluster.command_list);)
        DEBUG_ONLY(debug_draw.Begin(context);)
        DEBUG_ONLY(debug_shapes.Draw(debug_draw, camera_cluster.camera_uniforms_cpu->viewproj);)
        DEBUG_ONLY(debug_profile.Draw(profile, debug_draw, context.WindowWidth(), context.WindowHeight());)
    }

    Attachments GatherAttachments(CameraClusterDynamic& camera_cluster, const Camera::Target& target) {
        Array<SurfaceDynamic*, Camera::ColorAttachmentMaxCount > color_attachments;
        SurfaceDynamic* depth_stencil_attachment = nullptr;
        target.color_attachment_ids.ConstProcess([&](auto& color_attachment_id) {
            color_attachments.Add(bundle.Find<SurfaceDynamic>(color_attachment_id));
        });
        depth_stencil_attachment = bundle.Find<SurfaceDynamic>(target.depth_stencil_attachment_id);
        return Attachments(context, camera_cluster.command_list, color_attachments, depth_stencil_attachment, target.clear_color, target.clear_depth, target.last);
    }

    Array<SurfaceDynamic*, Camera::SurfaceMaxCount> GatherSurfaces(const Array<uint64, Camera::SurfaceMaxCount>& surface_ids) {
        Array<SurfaceDynamic*, Camera::SurfaceMaxCount> surfaces;
        surface_ids.ConstProcessIndex([&](auto& surface_id, unsigned index) {
            surfaces.Add(bundle.Find<SurfaceDynamic>(surface_id));
        });
        return surfaces;
    }

    void SetShaderAndSurfaces(CameraClusterDynamic& camera_cluster, CameraClusterDynamic* self_camera_cluster, const Attachments& attachments, ShaderDynamic& shader, Array<SurfaceDynamic*, Camera::SurfaceMaxCount>& surfaces, unsigned technique_index) {
        shader.Set(camera_cluster.command_list, technique_index);
        SurfaceDynamic::Set(context, camera_cluster.command_list, surfaces);
        ShaderDynamic::SetCameraConstants(camera_cluster.command_list, camera_cluster.camera_uniforms_gpu);
        if (self_camera_cluster)
            ShaderDynamic::SetBatchCameraConstants(camera_cluster.command_list, self_camera_cluster->camera_uniforms_gpu);
        const auto gpu = FillMisc(attachments);
        ShaderDynamic::SetMiscConstants(camera_cluster.command_list, gpu);
    }

    void SetMeshAndDraw(CameraClusterDynamic& camera_cluster, MeshDynamic& mesh, Uniforms& uniforms, uint64 gpu, unsigned instance_count) {
        ShaderDynamic::SetBatchInstanceConstants(camera_cluster.command_list, gpu);
        ShaderDynamic::SetUniformsConstants(camera_cluster.command_list, uniforms.Values());
        mesh.SetAndDraw(camera_cluster.command_list, instance_count);
    }

    struct MiscUniforms {
        Vector2 screen_size;
        Vector2 screen_size_inverse;
        Vector2 screen_scale;
        Vector2 screen_scale_inverse;
    };

    uint64 FillMisc(const Attachments& attachments) {
        uint64 gpu = 0;
#if !defined(__APPLE__) // TODO: Remove.
        MiscUniforms* cpu = nullptr;
        stack.Allocate(sizeof(MiscUniforms), (uint8*&)cpu, gpu);
        cpu->screen_size = attachments.ScreenSize(context.DynamicScale());
        cpu->screen_size_inverse = Vector2(1.f) / cpu->screen_size;
        cpu->screen_scale = context.DynamicScale();
        cpu->screen_scale_inverse = Vector2(1.f) / cpu->screen_scale;
#endif
        return gpu;
    }

    uint64 FillOne(CameraClusterDynamic& camera_cluster) {
        uint64 gpu = 0;
#if !defined(__APPLE__) // TODO: Remove.
        Instances* cpu = nullptr;
        stack.Allocate(sizeof(Instances), (uint8*&)cpu, gpu);
        (*cpu)[0].rotation = Quaternion();
        (*cpu)[0].position = camera_cluster.cluster->Batches()[0].Instances()[0].position;
        (*cpu)[0].scale = 1.f;
#endif
        return gpu;
    }

    uint64 FillBatch(CameraClusterDynamic& camera_cluster, const Batch& batch) {
        uint64 gpu = 0;
#if !defined(__APPLE__) // TODO: Remove.
        Instances* cpu = nullptr;
        stack.Allocate(sizeof(Instances), (uint8*&)cpu, gpu);
        batch.Instances().ConstProcessIndex([&](auto& instance, unsigned index) {
            (*cpu)[index] = instance;
        });
#endif
        return gpu;
    }

    void Time() {
        uint64 frame_total = 0;
        bundle.ProcessCameraClusters([&](auto& camera_cluster) {
            frame_total += camera_cluster.timings.Total();
        });
        gpu_frame_duration = frame_total;
    }

    class SortedCameraCluster { // TODO: Remove.
        CameraClusterDynamic* camera_cluster = nullptr;
        float priority = 0.f;

    public:
        SortedCameraCluster() {}
        SortedCameraCluster(CameraClusterDynamic* camera_cluster, float priority)
            : camera_cluster(camera_cluster), priority(priority) {}

        bool operator>(const SortedCameraCluster& other) const {
            return priority < other.priority;
        }

        void Push(Array<CommandList*, 16>& command_lists) {
            command_lists.Add(&camera_cluster->command_list);
        }
    };

    void Execute() {
        Array<SortedCameraCluster, 16> sorted_camera_clusters;
        bundle.ProcessCameraClusters([&](auto& camera_cluster) {
            if (auto* camera = bundle.Find<Camera>(camera_cluster.camera_id))
                sorted_camera_clusters.Add(&camera_cluster, camera->Priority());
        });
        sorted_camera_clusters.Sort();

        Array<CommandList*, 16> command_lists;
        sorted_camera_clusters.Process([&](auto& sorted_camera_cluster) {
            sorted_camera_cluster.Push(command_lists);
        });
        CommandList::Execute(context, command_lists);
    }

    void DrawDebug() {
        bundle.ProcessRenderClusters([&](auto& render_cluster) {
            DEBUG_ONLY(if (draw_bounds) {
                DrawDebugBox(-render_cluster.cluster->Bounds().radius, render_cluster.cluster->Bounds().radius, Quaternion(), render_cluster.cluster->Bounds().center, Vector3(0.f, 1.f, 0.f));
                DrawDebugAxis(Quaternion(), render_cluster.cluster->Bounds().center);
            })
                DEBUG_ONLY(if (draw_extents) {
                render_cluster.cluster->Batches().ConstProcess([&](auto& batch) {
                    batch.Instances().ConstProcessIndex([&](auto& instance, unsigned index) {
                        DrawDebugBox(-batch.Extents(), batch.Extents(), batch.Instances()[index].rotation, batch.Instances()[index].position, Vector3(0.5f, 0.5f, 0.5f));
                        DrawDebugAxis(Quaternion(), render_cluster.cluster->Bounds().center);
                    });
                });
            })
        });
    }

    Ray BuildRay(const Matrix& proj, const Matrix& view_inverse, float x, float y) const {
        const float px = (((2.f * x) / context.WindowWidth()) - 1.f) / proj.row0.x;
        const float py = (((-2.f * y) / context.WindowHeight()) + 1.f) / proj.row1.y;
        const auto from = view_inverse.Translation();
        const auto to = from + view_inverse.Rotation().Transform(Vector3(px, py, 1.f)) * 1000.f;
        return Ray(from, to);
    }

protected:
    Render() {}
    Render(const Parameters& parameters)
        : context(parameters), stack(&context) {
        DEBUG_ONLY(new(&debug_draw) DebugDraw(context);)
        LoadAll();
    }

    ~Render() {
        DEBUG_ONLY(context.Stop();)
        DEBUG_ONLY(UnloadAll();)
    }

    void Update() {
        Swap();
        DEBUG_ONLY(profile.BeginCPU(timer.Now(), Color::Blue);)
        DEBUG_ONLY(Render::DrawDebug();)
        UpdateCameras();
        ProcessCameras();
        Time();
        Execute();
        DEBUG_ONLY(profile.EndCPU(timer.Now());)
    }

    Id Pick(const Id& camera_id, float x, float y, unsigned _flags, Ray& out_ray) {
        if (auto* camera_cluster = bundle.FindCameraCluster(camera_id.cluster_id)) {
            out_ray = BuildRay(camera_cluster->camera_uniforms_cpu->proj, camera_cluster->camera_uniforms_cpu->view_inverse, x, y);
            Id out_id;
            bundle.ProcessRenderClusters([&](auto& render_cluster) {
                if (auto* flags = bundle.Find<Flags>(render_cluster.flags_id)) {
                    if (flags->Check(_flags, 0))
                        if (render_cluster.cluster->Bounds().Intersect(out_ray))
                            render_cluster.cluster->Batches().Process([&](auto& batch) {
                            Box box(batch.Extents());
                            batch.Instances().ProcessIndex([&](auto& instance, unsigned index) {
                                if (box.Intersect(out_ray, batch.Instances()[index].rotation, batch.Instances()[index].position))
                                    out_id = Id(render_cluster.cluster->Id(), batch.Id(), index);
                            });
                        });
                }
            });
            return out_id;
        }
        return Id();
    }

    void DrawClusterBox(const Id& id) {
        bundle.ProcessRenderClusters([&](auto& render_cluster) {
            if (const auto* batch = render_cluster.cluster->ConstFind(id.batch_id)) {
                DrawDebugBox(-batch->Extents(), batch->Extents(), batch->Instances()[id.instance_id].rotation.Normalize(), batch->Instances()[id.instance_id].position, Vector3(1.f, 0.f, 0.f));
            }
        });
    }

    void SwapSurface(const Id& id, unsigned index, uint64 texture_id) {
       const auto data_type = Data::DataTypeFromId(texture_id);
        if ((data_type == Data::Type::Surface) || (data_type == Data::Type::Surface)) {
            if (auto* render_cluster = bundle.FindRenderCluster(id.cluster_id))
                render_cluster->surface_ids[index] = texture_id;
        }
    }

    void SwapUniforms(const Id& id, uint64 uniforms_id) {
        const auto data_type = Data::DataTypeFromId(uniforms_id);
        if (data_type == Data::Type::Uniforms) {
            if (auto* cluster = bundle.FindCluster(id.cluster_id)) {
                cluster->Batches().ConstProcessIndex([&](auto& batch, unsigned batch_index) {
                    if (batch.Id() == id.batch_id)
                        if (auto* render_cluster = bundle.FindRenderCluster(id.cluster_id))
                            render_cluster->uniforms_ids[batch_index] = uniforms_id;
                });
            }
        }
    }

    void SwapPassUniforms(uint64 camera_id, uint32 target_id, uint32 technique_id, uint64 uniforms_id) {
        if (auto* camera = bundle.Find<Camera>(camera_id)) {
            camera->Targets().Process([&](auto& target) {
                if (target.target_id == target_id) {
                    target.passes.Process([&](auto& pass) {
                        if (pass.technique_id == technique_id) {
                            if (pass.auto_uniforms_id != uniforms_id) {
                                pass.auto_uniforms_id = uniforms_id;
                            }
                        }
                    });
                }
            });
        }
    }

    void SetUniform(uint64 uniforms_id, unsigned index, float value) {
        if (auto* uniforms = bundle.Find<Uniforms>(uniforms_id)) {
            if (index < 4 * Uniforms::UniformMaxCount) {
                ((float*)&uniforms->Values()[index / 4])[index % 4] = value;
            }
        }
    }

    void DrawDebugPoint(const Vector3& point, const Vector3& color) {
        DEBUG_ONLY(debug_shapes.DrawDebugPoint(point, color);)
    }

    void DrawDebugBox(const Vector3& min, const Vector3& max, const Quaternion& rotation, const Vector3& position, const Vector3& color) {
        DEBUG_ONLY(debug_shapes.DrawDebugBox(min, max, rotation, position, color);)
    }

    void DrawDebugAxis(const Quaternion& rotation, const Vector3& origin) {
        DEBUG_ONLY(debug_shapes.DrawDebugAxis(rotation, origin);)
    }

    void ToggleProfileJobs() { DEBUG_ONLY(debug_profile.ToggleProfileJobs();) }
    void ToggleDrawBounds() { DEBUG_ONLY(draw_bounds = !draw_bounds;) }
    void ToggleDrawExtents() { DEBUG_ONLY(draw_extents = !draw_extents;) }

    Vector2 GetDynamicScale() const { return context.DynamicScale(); }
    float GetAspectRatio() const { return context.AspectRatio(); }
    Vector2 GetWindowSize() const { return context.WindowSize(); }
    uint64 GetGPUFrameDuration() const { return gpu_frame_duration; }
    void SetDynamicScale(float scale) { context.SetDynamicScale(scale); }

    unsigned WindowWidth() const { return context.WindowWidth(); }
    unsigned WindowHeight() const { return context.WindowHeight(); }
};

class Control : public Render {
    Commands commands;

    void LoadAll() {
        bundle.Process([&](auto& data) {
            switch (Data::DataTypeFromId(data.Id())) {
            case Data::Type::Script: ((ScriptDynamic&)data).Load(bundle); break;
            default: break;
            }
        });
    }

    void UnloadAll() {
        bundle.Process([&](auto& data) {
            switch (Data::DataTypeFromId(data.Id())) {
            case Data::Type::Script: ((ScriptDynamic*)&data)->~ScriptDynamic(); break;
            default: break;
            }
        });
    }

    void Call(float elapsed_time) {
        bundle.ProcessScriptClusters([&](auto& script_cluster) {
            if (auto* script = bundle.Find<ScriptDynamic>(script_cluster.script_id))
                script->Execute(elapsed_time, WindowWidth(), WindowHeight());
        });
    }

    void Chase() {
        for (unsigned i = 0; i < Follow::BucketMaxCount; ++i) {
            bundle.ProcessFollowClusters([&](auto& follow_cluster) {
                follow_cluster.follow_ids.ProcessIndex([&](uint64 follow_id, unsigned index) {
                    if (auto* follow = bundle.Find<Follow>(follow_id))
                        if (follow->GetBucketIndex() == i)
                            if (auto* followed_follow_cluster = bundle.FindFollowCluster(follow->FollowedClusterId()))
                                if (auto* followed_cluster = bundle.FindCluster(follow->FollowedClusterId()))
                                    if (auto* followed_batch = followed_cluster->Find(follow->FollowedBatchId())) {
                                        follow->Update(follow_cluster.cluster->Batches()[index], *followed_batch);
                                        follow_cluster.cluster->ComputeBounds();
                                    }
                });
            });
        }
    }

    template<typename F> void Apply(const Id& id, F func) { // TODO: Remove.
        if (auto follow_cluster = bundle.FindFollowCluster(id.cluster_id))
            follow_cluster->follow_ids.ProcessIndex([&](uint64 follow_id, unsigned index) {
                auto& batch = follow_cluster->cluster->Batches()[index];
                if (batch.Id() == id.batch_id)
                    func(batch);
            });
    }

protected:
    Control() {}
    Control(const Parameters& parameters)
    : Render(parameters) {
        LoadAll();
    }

    ~Control() {
        DEBUG_ONLY(UnloadAll();)
    }

    void Init(const Commands& _commands) { // TODO: Remove.
        commands = _commands;
        bundle.ProcessScriptClusters([&](auto& script_cluster) {
            if (auto* script = bundle.Find<ScriptDynamic>(script_cluster.script_id))
                script->Init(commands);
        });
    }

    void Update() {
        DEBUG_ONLY(profile.BeginCPU(timer.Now(), Color::Red);)
        Call(timer.ElapsedTime());
        Chase();
        DEBUG_ONLY(profile.EndCPU(timer.Now());)
    }

    Quaternion GetRotation(const Id& id) {
        Quaternion quaternion;
        Apply(id, [&](auto& batch) {
            quaternion = batch.Instances()[id.instance_id].rotation;
        });
        return quaternion;
    }

    Vector3 GetPosition(const Id& id) {
        Vector3 position(0.f);
        Apply(id, [&](auto& batch) {
            position = batch.Instances()[id.instance_id].position;
        });
        return position;
    }

    void SetPosition(const Id& id, const Vector3& position) { // TODO: Missing cluster->ComputeBounds().
        Apply(id, [&](auto& batch) {
            batch.Instances()[id.instance_id].position = position;
        });
    }

    void SetRotation(const Id& id, const Quaternion& rotation) {
        Apply(id, [&](auto& batch) {
            batch.Instances()[id.instance_id].rotation = rotation;
        });
    }

    float GetValue(uint64 dictionary_id, uint32 hash) {
        float value = 0.0f;
        if (auto* dictionary = bundle.Find<Dictionary>(dictionary_id)) {
            return dictionary->Find(hash);
        }
        return value;
    }

public:
    void Mouse(MousePhase phase, Button button, float u, float v, float w) {
        bundle.ProcessScriptClusters([&](auto& script_cluster) {
            if (auto* script = bundle.Find<ScriptDynamic>(script_cluster.script_id))
                script->Mouse(phase, button, u, v, w);
        });
    }

    void Keyboard(KeyboardPhase phase, Key key) {
        bundle.ProcessScriptClusters([&](auto& script_cluster) {
            if (auto* script = bundle.Find<ScriptDynamic>(script_cluster.script_id))
                script->Keyboard(phase, key);
        });
    }
};

class Engine : public Control {

    static Engine* engine; // TODO: Remove.

public:
    Engine() {}
    Engine(const Parameters& parameters)
    : Control(parameters) {
        engine = this;

        Commands commands;
        commands.is_release = []() { DEBUG_ONLY(return false); return true; };
        commands.toggle_profile_jobs = []() { DEBUG_ONLY(engine->ToggleProfileJobs();) };
        commands.toggle_draw_bounds = []() { DEBUG_ONLY(engine->ToggleDrawBounds();) };
        commands.toggle_draw_extents = []() { DEBUG_ONLY(engine->ToggleDrawExtents();) };
        commands.data_id = [](const char* name) { return Data::IdFromName(name); };
        commands.get_value = [](uint64 dictionary_id, uint32 hash) { return engine->GetValue(dictionary_id, hash); };
        commands.get_dynamic_scale = []() { return engine->GetDynamicScale(); };
        commands.get_aspect_ratio = []() { return engine->GetAspectRatio(); };
        commands.get_window_size = []() { return engine->GetWindowSize(); };
        commands.get_gpu_frame_duration = []() { return engine->GetGPUFrameDuration(); };
        commands.set_dynamic_scale = [](float scale) { return engine->SetDynamicScale(scale); };
        commands.get_rotation = [](const Id& id) { return engine->GetRotation(id); };
        commands.get_position = [](const Id& id) { return engine->GetPosition(id); };
        commands.set_position = [](const Id& id, const Vector3& position) { engine->SetPosition(id, position); };
        commands.set_rotation = [](const Id& id, const Quaternion& rotation) { engine->SetRotation(id, rotation); };
        commands.pick = [](const Id& camera_id, float u, float v, unsigned flags, Ray& out_ray) { return engine->Pick(camera_id, u, v, flags, out_ray); };
        commands.swap_surface = [](const Id& id, unsigned index, uint64 texture_id) { engine->SwapSurface(id, index, texture_id); };
        commands.swap_uniforms = [](const Id& id, uint64 uniforms_id) { engine->SwapUniforms(id, uniforms_id); };
        commands.swap_pass_uniforms = [](uint64 camera_id, uint32 target_id, uint32 technique_id, uint64 uniforms_id) { engine->SwapPassUniforms(camera_id, target_id, technique_id, uniforms_id); };
        commands.set_uniform = [](uint64 uniforms_id, unsigned index, float value) { engine->SetUniform(uniforms_id, index, value); };
        commands.draw_cluster_box = [](const Id& id) { DEBUG_ONLY(engine->DrawClusterBox(id);) };
        commands.draw_point = [](const Vector3& point, const Vector3& color) { DEBUG_ONLY(engine->DrawDebugPoint(point, color);) };
        commands.play = [](const Id& id, uint32 sound_id, float volume) { return engine->Play(id, sound_id, volume); };
        Init(commands);
    }

    void Update() {
        timer.Tick();
        Control::Update();
        Audio::Update();
        Render::Update();
    }
};

Engine* Engine::engine = nullptr;
