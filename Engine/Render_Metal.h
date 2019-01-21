
static const NSUInteger MaxBuffersInFlight = 3; // TODO: Move to Context.

struct Parameters {
    Parameters(unsigned width, unsigned height, UIView* view)
    : width(width), height(height), view(view) {}

    unsigned width;
    unsigned height;
    UIView* view;
};

class Window {
    unsigned width = 8;
    unsigned height = 8;

public:
    Window() {}
    Window(const Parameters& parameters)
    : width(parameters.width), height(parameters.height) {
    }

    bool UpdateSize() {
        return false;
    }

    unsigned Width() const { return width; }
    unsigned Height() const { return height; }
};

class Context : public NoCopy {
public:
    static const unsigned BufferCount = 2;
    
private:
    Window window;
    MTKView* view;
    id <MTLDevice> device;
    id <MTLCommandQueue> command_queue;
    dispatch_semaphore_t semaphore;
    Vector2 dynamic_scale = Vector2(1.f);
    
public:
    Context() {}
    Context(const Parameters& parameters)
    : view((MTKView*)parameters.view), window(parameters) {
        view.device = MTLCreateSystemDefaultDevice();
        view.backgroundColor = UIColor.blackColor;
        view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
        view.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB; // TODO: Move to Attachment.
        view.sampleCount = 1;
        device = view.device;
        command_queue = [device newCommandQueue];
        semaphore = dispatch_semaphore_create(MaxBuffersInFlight);
    }
    
    void Stop() {
    }
    
    void Swap() {
    }
    
    unsigned WindowWidth() const { return window.Width(); }
    unsigned WindowHeight() const { return window.Height(); }
    float AspectRatio() const { return (float)window.Width() / (float)window.Height(); }
    Vector2 WindowSize() const { return Vector2((float)window.Width(), (float)window.Height()); }
    
    Vector2 DynamicScale() const { return dynamic_scale; }
    void SetDynamicScale(float scale) { dynamic_scale = scale; }

    MTKView* View() { return view; }
    id <MTLDevice> Device() { return device; }
    id <MTLCommandQueue> CommandQueue() { return command_queue; }
    const dispatch_semaphore_t& Semaphore() const { return semaphore; }
};

class CommandList {
    id <MTLCommandBuffer> command_buffer;
    id <MTLRenderCommandEncoder> command_encoder;

public:
    CommandList() {}
    CommandList(Context& context) {
    }

    void Reset(Context& context) {
        command_buffer = [context.CommandQueue() commandBuffer];
        command_buffer.label = @"Command Buffer";
    }

    void Begin(Context& context, bool last, MTLRenderPassDescriptor* pass_descriptor) {
        if (last) {
            dispatch_semaphore_wait(context.Semaphore(), DISPATCH_TIME_FOREVER);
        }
        command_encoder = [command_buffer renderCommandEncoderWithDescriptor:pass_descriptor];
        command_encoder.label = @"Render Command Encoder";
    }

    void End(Context& context, bool last) {
        [command_encoder endEncoding];
        if (last) {
            __block dispatch_semaphore_t block_sema = context.Semaphore();
            [command_buffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
                dispatch_semaphore_signal(block_sema); }];
            [command_buffer presentDrawable:context.View().currentDrawable];
        }
    }

    void Close(Context& context) {
        [command_buffer commit];
    }

    static void Execute(Context& context, Array<CommandList*, 16>& command_lists) {
    }
    
    bool Empty() const { return true; }

    id <MTLRenderCommandEncoder> Encoder() { return command_encoder; }
};

class Buffer : public NoCopy {
    uint64 gpu = 0;
    uint8* cpu = nullptr;
    
public:
    Buffer(Context& context, size size) {
    }
    
    uint64 GPU() const { return gpu; }
    uint8* CPU() const { return cpu; }
    
    static constexpr size AlignSize(size u) {
        return Math::AlignSize(u, (size)256);
    }
};

class MeshDynamic : public Mesh {
public:
    MeshDynamic(uint64 id, const String& name)
    : Mesh(id, name) {}
    
    void Load(Bundle& bundle, Context& context, CommandList& upload_list) {
    }
    
    void SetAndDraw(CommandList& command_list, unsigned instance_count) const {
    }
};

class SurfaceDynamic : public Surface {
    bool UpdateSize(Context& context) {
        unsigned w = sizes.x > 0.f ? (unsigned)sizes.x : context.WindowWidth();
        unsigned h = sizes.y > 0.f ? (unsigned)sizes.y : context.WindowHeight();
        w = resize > 0.f ? (unsigned)((float)w * resize) : w;
        h = resize > 0.f ? (unsigned)((float)h * resize) : h;
        if ((width == w) && (height == h))
            return false;
        width = w;
        height = h;
        return true;
    }

public:
    void Load(Bundle& bundle, Context& context, CommandList& upload_list) {
    }

    void Create(Context& context) {
    }

    void Destroy(Context& context) {
    }
    
    static void Set(Context& context, CommandList& command_list, Array<SurfaceDynamic*, Camera::SurfaceMaxCount>& surfaces) {
    }

    void Set(MTLRenderPassAttachmentDescriptor* attachment) {
        //attachment.texture = ;
        //attachment.loadAction = ;
        //attachment.storeAction = ;
    }

    void SetColor(MTLRenderPassColorAttachmentDescriptor* color_attachment) {
        color_attachment.clearColor = MTLClearColorMake(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        Set(color_attachment);
    }

    void SetDepth(MTLRenderPassDepthAttachmentDescriptor* depth_attachment) {
        depth_attachment.clearDepth = 0.0f;
        Set(depth_attachment);
    }

    Vector2 Size() const { return Vector2(width, height); }
    Vector2 ScreenSize(const Vector2& dynamic_scale) const { return Vector2(width, height) * (is_default ? 1.f : dynamic_scale); }
};

class Attachments : public NoCopy {
public:
    static const unsigned ColorMaxCount = 4;
    
    Attachments(Context& context, CommandList& command_list, const Array<SurfaceDynamic*, ColorMaxCount>& color_attachments, SurfaceDynamic* depth_stencil_attachment, bool clear_color, bool clear_depth, bool last)
    : context(context), command_list(command_list), last(last) {
        auto* pass_descriptor = [MTLRenderPassDescriptor renderPassDescriptor];
        color_attachments.ConstProcessIndex([&](const auto& color_attachment, unsigned index) {
            color_attachment->Set(pass_descriptor.colorAttachments[index]);
        });
        if (depth_stencil_attachment) {
            depth_stencil_attachment->Set(pass_descriptor.depthAttachment);
        }
        const auto screen_size = depth_stencil_attachment ? depth_stencil_attachment->Size() : color_attachments[0]->Size(); // TODO: Use dynamic scale?
        pass_descriptor.renderTargetWidth = screen_size.x;
        pass_descriptor.renderTargetHeight = screen_size.y;

        pass_descriptor = context.View().currentRenderPassDescriptor; // TODO: Remove.

        command_list.Begin(context, last, last ? context.View().currentRenderPassDescriptor : pass_descriptor);
    }
    
    ~Attachments() {
        command_list.End(context, last);
    }
    
    Vector2 ScreenSize(const Vector2& dynamic_scale) const {
        return Vector2(); // TODO
    }
    
private:
    Context& context;
    CommandList& command_list;
    bool last;
};

class ShaderDynamic : public Shader {
public:
    void Load(Bundle& bundle, Context& context) {
    }

    void Set(CommandList& command_list, const unsigned index) const {
    }
    
    static void SetBatchInstanceConstants(CommandList& command_list, uint64 gpu) {
    }
    
    static void SetUniformsConstants(CommandList& command_list, const Vector4* uniforms) {
    }
    
    static void SetBatchCameraConstants(CommandList& command_list, uint64 gpu) {
    }
    
    static void SetCameraConstants(CommandList& command_list, uint64 gpu) {
    }
    
    static void SetMiscConstants(CommandList& command_list, uint64 gpu) {
    }

    
private:
    class TechniqueDynamic : public NoCopy {
    public:
        void Load(Context& context, const Shader& shader, const uint8* data) {
        }
        
        void Set(CommandList& command_list) const {
        }
    };
};

class Timings : public NoCopy {
public:
    static const unsigned BufferCount = Context::BufferCount + 1;
    
private:
    static const Color TimingsColor = Color::Aqua;
    static const unsigned TimingsMaxCount = 64;

public:
    Timings() {}
    Timings(Context& context) {
    }
    
    void Swap(Context& context, uint64 now) {
    }
    
    void Push(CommandList& command_list) {
    }
    
    void Query(CommandList& command_list) {
    }
    
    void GatherBegin() {
    }
    
    void GatherOne(Profile& profile, Color color) {
    }
    
    void GatherEnd() {
    }
    
    uint64 Total() const { return 0; }
};
