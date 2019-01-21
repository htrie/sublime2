
struct Range : public D3D12_RANGE {
    Range(SIZE_T begin,SIZE_T end) {
        Begin = begin;
        End = end;
    }
};

struct ClearValue : public D3D12_CLEAR_VALUE {
    ClearValue(DXGI_FORMAT format, const FLOAT color[4]) {
        Format = format;
        memcpy(Color, color, sizeof(Color));
    }
};

struct ShaderBytecode : public D3D12_SHADER_BYTECODE {
    ShaderBytecode(ID3DBlob* pShaderBlob) {
        pShaderBytecode = pShaderBlob->GetBufferPointer();
        BytecodeLength = pShaderBlob->GetBufferSize();
    }
    ShaderBytecode(const void* _pShaderBytecode, SIZE_T bytecodeLength) {
        pShaderBytecode = _pShaderBytecode;
        BytecodeLength = bytecodeLength;
    }
};

struct CPUDescriptorHandle : public D3D12_CPU_DESCRIPTOR_HANDLE {
    CPUDescriptorHandle() {}
    CPUDescriptorHandle(const D3D12_CPU_DESCRIPTOR_HANDLE &other, INT offsetInDescriptors, UINT descriptorIncrementSize) {
        ptr = other.ptr + offsetInDescriptors * descriptorIncrementSize;
    }
};

struct GPUDescriptorHandle : public D3D12_GPU_DESCRIPTOR_HANDLE {
    GPUDescriptorHandle(const D3D12_GPU_DESCRIPTOR_HANDLE &other, INT offsetInDescriptors, UINT descriptorIncrementSize) {
        ptr = other.ptr + offsetInDescriptors * descriptorIncrementSize;
    }
};

struct HeapProperties : public D3D12_HEAP_PROPERTIES {
    explicit HeapProperties(D3D12_HEAP_TYPE type, UINT creationNodeMask = 1, UINT nodeMask = 1) {
        Type = type;
        CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        CreationNodeMask = creationNodeMask;
        VisibleNodeMask = nodeMask;
    }
};

struct TextureCopyLocation : public D3D12_TEXTURE_COPY_LOCATION {
    TextureCopyLocation(ID3D12Resource* pRes, D3D12_PLACED_SUBRESOURCE_FOOTPRINT const& Footprint) {
        pResource = pRes;
        Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        PlacedFootprint = Footprint;
    }
    TextureCopyLocation(ID3D12Resource* pRes, UINT Sub) {
        pResource = pRes;
        Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        SubresourceIndex = Sub;
    }
};

struct ResourceDesc : public D3D12_RESOURCE_DESC {
    ResourceDesc(D3D12_RESOURCE_DIMENSION dimension, UINT64 alignment, UINT64 width, UINT height, UINT16 depthOrArraySize, UINT16 mipLevels, DXGI_FORMAT format, UINT sampleCount, UINT sampleQuality, D3D12_TEXTURE_LAYOUT layout, D3D12_RESOURCE_FLAGS flags) {
        Dimension = dimension;
        Alignment = alignment;
        Width = width;
        Height = height;
        DepthOrArraySize = depthOrArraySize;
        MipLevels = mipLevels;
        Format = format;
        SampleDesc.Count = sampleCount;
        SampleDesc.Quality = sampleQuality;
        Layout = layout;
        Flags = flags;
    }

    static ResourceDesc Buffer(const D3D12_RESOURCE_ALLOCATION_INFO& resAllocInfo, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE) {
        return ResourceDesc(D3D12_RESOURCE_DIMENSION_BUFFER, resAllocInfo.Alignment, resAllocInfo.SizeInBytes, 1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, flags);
    }
    static ResourceDesc Buffer(UINT64 width, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE, UINT64 alignment = 0) {
        return ResourceDesc(D3D12_RESOURCE_DIMENSION_BUFFER, alignment, width, 1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, flags);
    }
    static ResourceDesc Tex1D(DXGI_FORMAT format, UINT64 width, UINT16 arraySize = 1, UINT16 mipLevels = 0, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN, UINT64 alignment = 0) {
        return ResourceDesc(D3D12_RESOURCE_DIMENSION_TEXTURE1D, alignment, width, 1, arraySize, mipLevels, format, 1, 0, layout, flags);
    }
    static ResourceDesc Tex2D(DXGI_FORMAT format, UINT64 width, UINT height, UINT16 arraySize = 1, UINT16 mipLevels = 0, UINT sampleCount = 1, UINT sampleQuality = 0, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN, UINT64 alignment = 0) {
        return ResourceDesc(D3D12_RESOURCE_DIMENSION_TEXTURE2D, alignment, width, height, arraySize, mipLevels, format, sampleCount, sampleQuality, layout, flags);
    }
    static ResourceDesc Tex3D(DXGI_FORMAT format, UINT64 width, UINT height, UINT16 depth, UINT16 mipLevels = 0, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN, UINT64 alignment = 0) {
        return ResourceDesc(D3D12_RESOURCE_DIMENSION_TEXTURE3D, alignment, width, height, depth, mipLevels, format, 1, 0, layout, flags);
    }
};

struct DescriptorRange : public D3D12_DESCRIPTOR_RANGE {
    DescriptorRange() {}
    DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT numDescriptors, UINT baseShaderRegister, UINT registerSpace = 0, UINT offsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND) {
        RangeType = rangeType;
        NumDescriptors = numDescriptors;
        BaseShaderRegister = baseShaderRegister;
        RegisterSpace = registerSpace;
        OffsetInDescriptorsFromTableStart = offsetInDescriptorsFromTableStart;
    }
};

struct RootDescriptorTable : public D3D12_ROOT_DESCRIPTOR_TABLE {
    RootDescriptorTable(UINT numDescriptorRanges, const D3D12_DESCRIPTOR_RANGE* _pDescriptorRanges) {
        NumDescriptorRanges = numDescriptorRanges;
        pDescriptorRanges = _pDescriptorRanges;
    }
};

struct RootConstants : public D3D12_ROOT_CONSTANTS {
    RootConstants(UINT num32BitValues, UINT shaderRegister, UINT registerSpace = 0) {
        Num32BitValues = num32BitValues;
        ShaderRegister = shaderRegister;
        RegisterSpace = registerSpace;
    }
};

struct RootDescriptor : public D3D12_ROOT_DESCRIPTOR {
    RootDescriptor(UINT shaderRegister, UINT registerSpace = 0) {
        ShaderRegister = shaderRegister;
        RegisterSpace = registerSpace;
    }
};

struct RootParameter : public D3D12_ROOT_PARAMETER {
    void InitAsDescriptorTable(UINT numDescriptorRanges, const D3D12_DESCRIPTOR_RANGE* pDescriptorRanges, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) {
        ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        ShaderVisibility = visibility;
        DescriptorTable = RootDescriptorTable(numDescriptorRanges, pDescriptorRanges);
    }

    void InitAsConstants(UINT num32BitValues, UINT shaderRegister, UINT registerSpace = 0, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) {
        ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        ShaderVisibility = visibility;
        Constants = RootConstants(num32BitValues, shaderRegister, registerSpace);
    }

    void InitAsConstantBufferView(UINT shaderRegister, UINT registerSpace = 0, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) {
        ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        ShaderVisibility = visibility;
        Descriptor = RootDescriptor(shaderRegister, registerSpace);
    }

    void InitAsShaderResourceView(UINT shaderRegister, UINT registerSpace = 0, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) {
        ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
        ShaderVisibility = visibility;
        Descriptor = RootDescriptor(shaderRegister, registerSpace);
    }

    void InitAsUnorderedAccessView(UINT shaderRegister, UINT registerSpace = 0, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) {
        ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
        ShaderVisibility = visibility;
        Descriptor = RootDescriptor(shaderRegister, registerSpace);
    }
};

struct StaticSamplerDesc : public D3D12_STATIC_SAMPLER_DESC {
    StaticSamplerDesc() {}
    StaticSamplerDesc(UINT shaderRegister, D3D12_FILTER filter = D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE addressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE addressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE addressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP, FLOAT mipLODBias = 0, UINT maxAnisotropy = 16, D3D12_COMPARISON_FUNC comparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_STATIC_BORDER_COLOR borderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE, FLOAT minLOD = 0.f, FLOAT maxLOD = D3D12_FLOAT32_MAX, D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL, UINT registerSpace = 0) {
        ShaderRegister = shaderRegister;
        Filter = filter;
        AddressU = addressU;
        AddressV = addressV;
        AddressW = addressW;
        MipLODBias = mipLODBias;
        MaxAnisotropy = maxAnisotropy;
        ComparisonFunc = comparisonFunc;
        BorderColor = borderColor;
        MinLOD = minLOD;
        MaxLOD = maxLOD;
        ShaderVisibility = shaderVisibility;
        RegisterSpace = registerSpace;
    }
};

struct RootSignatureDesc : public D3D12_ROOT_SIGNATURE_DESC {
    RootSignatureDesc( UINT numParameters, const D3D12_ROOT_PARAMETER* _pParameters,UINT numStaticSamplers = 0, const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = NULL, D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE) {
        NumParameters = numParameters;
        pParameters = _pParameters;
        NumStaticSamplers = numStaticSamplers;
        pStaticSamplers = _pStaticSamplers;
        Flags = flags;
    }
};

struct ResourceBarrier : public D3D12_RESOURCE_BARRIER {
    static ResourceBarrier Transition(ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE) {
        ResourceBarrier result;
        ZeroMemory(&result, sizeof(result));
        D3D12_RESOURCE_BARRIER &barrier = result;
        result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        result.Flags = flags;
        barrier.Transition.pResource = pResource;
        barrier.Transition.StateBefore = stateBefore;
        barrier.Transition.StateAfter = stateAfter;
        barrier.Transition.Subresource = subresource;
        return result;
    }

    static ResourceBarrier Aliasing(ID3D12Resource* pResourceBefore, ID3D12Resource* pResourceAfter) {
        ResourceBarrier result;
        ZeroMemory(&result, sizeof(result));
        D3D12_RESOURCE_BARRIER &barrier = result;
        result.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
        barrier.Aliasing.pResourceBefore = pResourceBefore;
        barrier.Aliasing.pResourceAfter = pResourceAfter;
        return result;
    }

    static ResourceBarrier UAV(ID3D12Resource* pResource) {
        ResourceBarrier result;
        ZeroMemory(&result, sizeof(result));
        D3D12_RESOURCE_BARRIER &barrier = result;
        result.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier.UAV.pResource = pResource;
        return result;
    }
};

struct RenderTargetBlendDesc : public D3D12_RENDER_TARGET_BLEND_DESC {
    explicit RenderTargetBlendDesc() {
        BlendEnable = FALSE;
        LogicOpEnable = FALSE;
        SrcBlend = D3D12_BLEND_ONE;
        DestBlend = D3D12_BLEND_ZERO;
        BlendOp = D3D12_BLEND_OP_ADD;
        SrcBlendAlpha = D3D12_BLEND_ONE;
        DestBlendAlpha = D3D12_BLEND_ZERO;
        BlendOpAlpha = D3D12_BLEND_OP_ADD;
        LogicOp = D3D12_LOGIC_OP_NOOP;
        RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    }
};

struct BlendDesc : public D3D12_BLEND_DESC {
    explicit BlendDesc() {
        AlphaToCoverageEnable = FALSE;
        IndependentBlendEnable = FALSE;
        for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
            RenderTarget[i] = RenderTargetBlendDesc();
    }
};

struct RasterizerDesc : public D3D12_RASTERIZER_DESC {
    explicit RasterizerDesc() {
        FillMode = D3D12_FILL_MODE_SOLID;
        CullMode = D3D12_CULL_MODE_BACK;
        FrontCounterClockwise = FALSE;
        DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        DepthClipEnable = TRUE;
        MultisampleEnable = FALSE;
        AntialiasedLineEnable = FALSE;
        ForcedSampleCount = 0;
        ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    }

    explicit RasterizerDesc(D3D12_FILL_MODE fillMode, D3D12_CULL_MODE cullMode, BOOL frontCounterClockwise, INT depthBias, FLOAT depthBiasClamp, FLOAT slopeScaledDepthBias, BOOL depthClipEnable, BOOL multisampleEnable, BOOL antialiasedLineEnable, UINT forcedSampleCount, D3D12_CONSERVATIVE_RASTERIZATION_MODE conservativeRaster) {
        FillMode = fillMode;
        CullMode = cullMode;
        FrontCounterClockwise = frontCounterClockwise;
        DepthBias = depthBias;
        DepthBiasClamp = depthBiasClamp;
        SlopeScaledDepthBias = slopeScaledDepthBias;
        DepthClipEnable = depthClipEnable;
        MultisampleEnable = multisampleEnable;
        AntialiasedLineEnable = antialiasedLineEnable;
        ForcedSampleCount = forcedSampleCount;
        ConservativeRaster = conservativeRaster;
    }
};

struct DepthStencilDesc : public D3D12_DEPTH_STENCIL_DESC {
    explicit DepthStencilDesc() {
        DepthEnable = TRUE;
        DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        StencilEnable = FALSE;
        StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
        StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
        { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
        FrontFace = defaultStencilOp;
        BackFace = defaultStencilOp;
    }

    explicit DepthStencilDesc(BOOL depthEnable, D3D12_DEPTH_WRITE_MASK depthWriteMask, D3D12_COMPARISON_FUNC depthFunc, BOOL stencilEnable, UINT8 stencilReadMask, UINT8 stencilWriteMask, D3D12_STENCIL_OP frontStencilFailOp, D3D12_STENCIL_OP frontStencilDepthFailOp, D3D12_STENCIL_OP frontStencilPassOp, D3D12_COMPARISON_FUNC frontStencilFunc, D3D12_STENCIL_OP backStencilFailOp, D3D12_STENCIL_OP backStencilDepthFailOp, D3D12_STENCIL_OP backStencilPassOp, D3D12_COMPARISON_FUNC backStencilFunc) {
        DepthEnable = depthEnable;
        DepthWriteMask = depthWriteMask;
        DepthFunc = depthFunc;
        StencilEnable = stencilEnable;
        StencilReadMask = stencilReadMask;
        StencilWriteMask = stencilWriteMask;
        FrontFace.StencilFailOp = frontStencilFailOp;
        FrontFace.StencilDepthFailOp = frontStencilDepthFailOp;
        FrontFace.StencilPassOp = frontStencilPassOp;
        FrontFace.StencilFunc = frontStencilFunc;
        BackFace.StencilFailOp = backStencilFailOp;
        BackFace.StencilDepthFailOp = backStencilDepthFailOp;
        BackFace.StencilPassOp = backStencilPassOp;
        BackFace.StencilFunc = backStencilFunc;
    }
};

class DescriptorPool : public NoCopy {
    static const UINT MaxCount = 256;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap;
    UINT descriptor_size;
    FixedArray<UINT, MaxCount> free_list;
    UINT head;

public:
    DescriptorPool() {}
    DescriptorPool(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags) {
        D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
        heap_desc.NumDescriptors = MaxCount;
        heap_desc.Type = type;
        heap_desc.Flags = flags;
        device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&heap));
        descriptor_size = device->GetDescriptorHandleIncrementSize(type);
        for (unsigned i = 0; i < MaxCount - 1; ++i)
            free_list[i] = i + 1;
        free_list[MaxCount - 1] = (UINT)-1;
        head = 0;
    }

    UINT Allocate() {
        UINT index = head;
        head = free_list[head];
        return index;
    }

    void Free(UINT index) {
        free_list[index] = head;
        head = index;
    }

    CPUDescriptorHandle CPUHandle(UINT index) {
        return CPUDescriptorHandle(heap->GetCPUDescriptorHandleForHeapStart(), index, descriptor_size);
    }

    GPUDescriptorHandle GPUHandle(UINT index) {
        return GPUDescriptorHandle(heap->GetGPUDescriptorHandleForHeapStart(), index, descriptor_size);
    }

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> Heap() { return heap; }
};

struct Parameters {
    Parameters(WNDPROC proc, HINSTANCE hInstance, const char* name, void* data, int nCmdShow, unsigned width, unsigned height)
        : proc(proc), hInstance(hInstance), name(name), data(data), nCmdShow(nCmdShow), width(width), height(height) {}

    WNDPROC proc;
    HINSTANCE hInstance;
    const char* name = nullptr;
    void* data = nullptr;
    int nCmdShow;
    unsigned width;
    unsigned height;
};

class Window {
    UINT width = 8;
    UINT height = 8;
    HWND hWnd = nullptr;

    void Register(const Parameters& parameters) {
        WNDCLASSEX window_class = { 0 };
        window_class.cbSize = sizeof(WNDCLASSEX);
        window_class.style = CS_HREDRAW | CS_VREDRAW;
        window_class.lpfnWndProc = parameters.proc;
        window_class.hInstance = parameters.hInstance;
        window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
        window_class.lpszClassName = parameters.name;
        window_class.hbrBackground = CreateSolidBrush(0x00000000);
        RegisterClassEx(&window_class);
    }

public:
    Window() {}
    Window(const Parameters& parameters) {
        Register(parameters);
        RECT rect = { 0, 0, static_cast<LONG>(parameters.width), static_cast<LONG>(parameters.height) };
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
        hWnd = CreateWindow(parameters.name, parameters.name, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, parameters.hInstance, parameters.data);
        ShowWindow(hWnd, parameters.nCmdShow);
    }

    bool UpdateSize() {
        const UINT old_width = width;
        const UINT old_height = height;
        RECT rect = {};
        GetClientRect(hWnd, &rect);
        width = Math::Max((LONG)8, rect.right - rect.left);
        height = Math::Max((LONG)8, rect.bottom - rect.top);
        return (old_width != width) || (old_height != height);
    }

    HWND Hwnd() const { return hWnd; };
    UINT Width() const { return width; }
    UINT Height() const { return height; }
};

class Context : public NoCopy {
public:
    static const unsigned BufferCount = 2;

private:
    Window window;
    DescriptorPool rtv_pool;
    DescriptorPool dsv_pool;
    DescriptorPool cbv_srv_uav_pool;
    Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
    Microsoft::WRL::ComPtr<ID3D12Device> device;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> swap_chain;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue;
    Microsoft::WRL::ComPtr<ID3D12Resource> default_texture_buffer;
    Microsoft::WRL::ComPtr<ID3D12Fence> fence;
    FixedArray<UINT64, BufferCount> fence_values;
    HANDLE fence_event = INVALID_HANDLE_VALUE;
    UINT default_texture_index = 0;
    UINT frame_index = 0;
    Vector2 dynamic_scale = Vector2(1.f);

    void InitDevice() {
        UINT dxgi_factory_flags = 0;
        DEBUG_ONLY(dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;)
        CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&factory));

        Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
        for (UINT i = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(i, &adapter); ++i) {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                continue;
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_1, _uuidof(ID3D12Device), nullptr)))
                break;
        }

        CheckResult(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&device)));
        DEBUG_ONLY(device->SetName(L"Device"));
    }

    void InitCommandQueue() {
        const auto queue_desc = QueueDesc();
        CheckResult(device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue)));
        DEBUG_ONLY(command_queue->SetName(L"Command Queue"));
    }

    void InitSwapChain() {
        const auto swap_chain_desc = SwapChainDesc(window.Width(), window.Height());
        Microsoft::WRL::ComPtr<IDXGISwapChain1> swap_chain1;
        CheckResult(factory->CreateSwapChainForHwnd(command_queue.Get(), window.Hwnd(), &swap_chain_desc, nullptr, nullptr, &swap_chain1));
        factory->MakeWindowAssociation(window.Hwnd(), DXGI_MWA_NO_ALT_ENTER);
        swap_chain1.As(&swap_chain);
    }

    void InitDefaultTexture() {
        const auto resource_desc = ResourceDesc::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_NONE);
        CheckResult(device->CreateCommittedResource(&HeapProperties(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&default_texture_buffer)));
        DEBUG_ONLY(Context::SetDebugName(default_texture_buffer.Get(), String("Default Texture Buffer"));)

        default_texture_index = cbv_srv_uav_pool.Allocate();
        const auto srv_desc = DefaultResourceViewDesc();
        device->CreateShaderResourceView(default_texture_buffer.Get(), &srv_desc, cbv_srv_uav_pool.CPUHandle(default_texture_index));
    }

    void InitPools() {
        new (&rtv_pool) DescriptorPool(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
        new (&dsv_pool) DescriptorPool(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
        new (&cbv_srv_uav_pool) DescriptorPool(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    }

    void InitFence() {
        CheckResult(device->CreateFence(fence_values[frame_index], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
        fence_values[frame_index]++;
        fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    }

    static DXGI_SWAP_CHAIN_DESC1 SwapChainDesc(unsigned width, unsigned height) {
        DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
        swap_chain_desc.BufferCount = BufferCount;
        swap_chain_desc.Width = width;
        swap_chain_desc.Height = height;
        swap_chain_desc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
        swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swap_chain_desc.SampleDesc.Count = 1;
        swap_chain_desc.Flags = 0;
        return swap_chain_desc;
    }

    static D3D12_COMMAND_QUEUE_DESC QueueDesc() {
        D3D12_COMMAND_QUEUE_DESC queue_desc = {};
        queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        return queue_desc;
    }

    static D3D12_SHADER_RESOURCE_VIEW_DESC DefaultResourceViewDesc() {
        D3D12_SHADER_RESOURCE_VIEW_DESC shader_resource_view = {};
        shader_resource_view.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        shader_resource_view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        shader_resource_view.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        shader_resource_view.Texture2D.PlaneSlice = 0;
        shader_resource_view.Texture2D.MipLevels = 1;
        shader_resource_view.Texture2D.MostDetailedMip = 0;
        shader_resource_view.Texture2D.ResourceMinLODClamp = 0.f;
        return shader_resource_view;
    }

    void Resize() {
        DXGI_SWAP_CHAIN_DESC desc = {};
        swap_chain->GetDesc(&desc);
        CheckResult(swap_chain->ResizeBuffers(BufferCount, window.Width(), window.Height(), desc.BufferDesc.Format, desc.Flags));
    }

    void Present() {
        swap_chain->Present(1, 0);
    }

    uint64 Signal() {
        uint64 fence_value = fence_values[frame_index];
        if (command_queue)
            command_queue->Signal(fence.Get(), fence_value);
        return fence_value;
    }

    void Next() {
        frame_index = swap_chain->GetCurrentBackBufferIndex();
    }

    void Wait() {
        if (fence) {
            fence->SetEventOnCompletion(fence_values[frame_index], fence_event);
            WaitForSingleObjectEx(fence_event, INFINITE, FALSE);
        }
    }

    void WaitGPUPrevious(uint64 fence_value) {
        if (fence->GetCompletedValue() < fence_values[frame_index]) {
            Wait();
        }
        fence_values[frame_index] = fence_value + 1;
    }

    void WaitGPUCurrent() {
        Wait();
        fence_values[frame_index]++;
        for (UINT n = 0; n < BufferCount; n++)
            fence_values[n] = fence_values[frame_index];
    }

public:
    static void CheckError(ID3DBlob* error) {
        if (error) {
            DEBUG_ONLY(OutputDebugStringA((char*)error->GetBufferPointer());)
            DEBUG_ONLY(throw Exception((char*)error->GetBufferPointer());)
        }
    }

    static void CheckResult(HRESULT hresult) {
        DEBUG_ONLY(if (FAILED(hresult)) throw Exception();)
    }

    static void EnableDebug() {
        Microsoft::WRL::ComPtr<ID3D12Debug> debug;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)))) {
            debug->EnableDebugLayer();
        }
    }

    static void Check() {
        Microsoft::WRL::ComPtr<IDXGIDebug1> dxgi_debug;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgi_debug)))) {
            dxgi_debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
        }
    }

    static void SetDebugName(ID3D12Object* object, const String& name) {
        wchar_t wide_name[String::MaxSize];
        if (MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, name.Data(), -1, wide_name, String::MaxSize) == 0) throw Exception();
        object->SetName(wide_name);
    }

    Context() {}
    Context(const Parameters& parameters)
    : window(parameters) {
        DEBUG_ONLY(EnableDebug());
        InitDevice();
        InitCommandQueue();
        InitSwapChain();
        InitPools();
        InitFence();
        InitDefaultTexture();
    }

    void Stop() {
        Signal();
        WaitGPUCurrent();
    }

    void Swap() {
        Present();
        uint64 fence_value = Signal();
        if (window.UpdateSize()) {
            WaitGPUCurrent();
            Resize();
            Next();
        } else {
            Next();
            WaitGPUPrevious(fence_value);
        }
    }

    void UpdateSubresources(ID3D12GraphicsCommandList* command_list, ID3D12Resource* dest_resource, ID3D12Resource* src_resource, UINT64 base_offset, UINT first, UINT count, const D3D12_SUBRESOURCE_DATA* src_data) {
        FixedArray<D3D12_PLACED_SUBRESOURCE_FOOTPRINT, 64> layouts;
        FixedArray<UINT64, 64> row_sizes;
        FixedArray<UINT, 64> num_rows;
        D3D12_RESOURCE_DESC desc = dest_resource->GetDesc();
        UINT64 required_size = 0;
        device->GetCopyableFootprints(&desc, first, count, base_offset, layouts.Values(), num_rows.Values(), row_sizes.Values(), &required_size);

        BYTE* data;
        CheckResult(src_resource->Map(0, NULL, (void**)(&data)));

        for (UINT i = 0; i < count; ++i) {
            D3D12_MEMCPY_DEST dest = { data + layouts[i].Offset, layouts[i].Footprint.RowPitch, layouts[i].Footprint.RowPitch * num_rows[i] };
            
            for (UINT z = 0; z < layouts[i].Footprint.Depth; ++z) {
                auto* dest_slice = (BYTE*)(dest.pData) + dest.SlicePitch * z;
                const auto* src_slice = (BYTE*)(src_data[i].pData) + src_data[i].SlicePitch * z;
                for (UINT y = 0; y < num_rows[i]; ++y) {
                    memcpy(dest_slice + dest.RowPitch * y, src_slice + src_data[i].RowPitch * y, row_sizes[i]);
                }
            }

            TextureCopyLocation dst(dest_resource, i + first);
            TextureCopyLocation src(src_resource, layouts[i]);
            command_list->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
        }

        src_resource->Unmap(0, NULL);
    }

    unsigned WindowWidth() const { return window.Width(); }
    unsigned WindowHeight() const { return window.Height(); }
    float AspectRatio() const { return (float)window.Width() / (float)window.Height(); }
    Vector2 WindowSize() const { return Vector2((float)window.Width(), (float)window.Height()); }

    Vector2 DynamicScale() const { return dynamic_scale; }
    void SetDynamicScale(float scale) { dynamic_scale = scale; }

    DescriptorPool& RTVPool() { return rtv_pool; }
    DescriptorPool& DSVPool() { return dsv_pool; }
    DescriptorPool& CBVPool() { return cbv_srv_uav_pool; }
    DescriptorPool& SRVPool() { return cbv_srv_uav_pool; }
    DescriptorPool& UAVPool() { return cbv_srv_uav_pool; }

    Microsoft::WRL::ComPtr<ID3D12Device> Device() { return device; }
    Microsoft::WRL::ComPtr<IDXGISwapChain3>& SwapChain() { return swap_chain; };
    Microsoft::WRL::ComPtr<ID3D12CommandQueue>& CommandQueue() { return command_queue; };

    UINT DefaultTextureIndex() const { return default_texture_index; }
    UINT FrameIndex() const { return frame_index; }
};

class CommandList : public NoCopy {
    FixedArray<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>, Context::BufferCount> command_allocators;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list;

public:
    CommandList() {}
    CommandList(Context& context) {
        for (UINT n = 0; n < Context::BufferCount; n++) {
            Context::CheckResult(context.Device()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocators[n])));
            DEBUG_ONLY(command_allocators[n]->SetName(L"Command Allocator"));
        }
    }
    
    void Reset(Context& context) {
        Context::CheckResult(command_allocators[context.FrameIndex()]->Reset());
        Context::CheckResult(context.Device()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocators[context.FrameIndex()].Get(), nullptr, IID_PPV_ARGS(&command_list)));
        DEBUG_ONLY(command_list->SetName(L"Command List"));
        Array<ID3D12DescriptorHeap*, 1> heaps;
        heaps.Add(context.CBVPool().Heap().Get());
        command_list->SetDescriptorHeaps(heaps.UsedCount(), heaps.Values());
    }

    void Close(Context& context) {
        if (command_list.Get() != nullptr)
            Context::CheckResult(command_list->Close());
    }

    static void Execute(Context& context, Array<CommandList*, 16>& command_lists) {
        Array<ID3D12CommandList*, 16> native_command_lists;
        command_lists.ConstProcess([&](auto* command_list) {
            if (auto native_command_list = command_list->Native().Get())
                native_command_lists.Add(native_command_list);
        });
        context.CommandQueue()->ExecuteCommandLists((UINT)native_command_lists.UsedCount(), native_command_lists.Values());
    }
 
    bool Empty() const { return command_list.Get() == nullptr; }

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& Native() { return command_list; }
};

class Buffer : public NoCopy {
    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    uint64 gpu = 0;
    uint8* cpu = nullptr;

public:
    Buffer() {}
    Buffer(Context& context, size size) {
        Context::CheckResult(context.Device()->CreateCommittedResource(&HeapProperties(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &ResourceDesc::Buffer(size), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&(resource))));
        DEBUG_ONLY(resource->SetName(L"Buffer"));

        Context::CheckResult(resource->Map(0, &Range(0, 0), (void**)&cpu));
        gpu = resource->GetGPUVirtualAddress();
    }

    static constexpr size AlignSize(size u) {
        return Math::AlignSize(u, (size)D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
    }

    uint64 GPU() const { return gpu; }
    uint8* CPU() const { return cpu; }
};

class MeshDynamic : public Mesh {
    Microsoft::WRL::ComPtr<ID3D12Resource> upload_buffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> vertex_buffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> index_buffer;
    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
    D3D12_INDEX_BUFFER_VIEW index_buffer_view;

    static DXGI_FORMAT Format(Attribute::Type type) {
        switch (type) {
        case Attribute::Type::U16: return DXGI_FORMAT_R16_UINT;
        case Attribute::Type::U32: return DXGI_FORMAT_R32_UINT;
        default: return DXGI_FORMAT_UNKNOWN;
        };
    }
 
    void CreateBuffers(Context& context, size vb_size, size ib_size) {
        Context::CheckResult(context.Device()->CreateCommittedResource(&HeapProperties(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &ResourceDesc::Buffer(vb_size + ib_size), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload_buffer)));
        Context::CheckResult(context.Device()->CreateCommittedResource(&HeapProperties(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &ResourceDesc::Buffer(vb_size), D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&vertex_buffer)));
        Context::CheckResult(context.Device()->CreateCommittedResource(&HeapProperties(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &ResourceDesc::Buffer(ib_size), D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&index_buffer)));
        DEBUG_ONLY(Context::SetDebugName(upload_buffer.Get(), String("Mesh ") + Name() + String(" Upload Buffer"));)
        DEBUG_ONLY(Context::SetDebugName(vertex_buffer.Get(), String("Mesh ") + Name() + String(" Vertex Buffer"));)
        DEBUG_ONLY(Context::SetDebugName(index_buffer.Get(), String("Mesh ") + Name() + String(" Index Buffer"));)
    }

    void CreateBufferViews(size vb_size, size ib_size) {
        vertex_buffer_view.BufferLocation = vertex_buffer->GetGPUVirtualAddress();
        vertex_buffer_view.SizeInBytes = (UINT)vb_size;
        vertex_buffer_view.StrideInBytes = vertices.Stride();
        index_buffer_view.BufferLocation = index_buffer->GetGPUVirtualAddress();
        index_buffer_view.SizeInBytes = (UINT)ib_size;
        index_buffer_view.Format = Format(indices.AttributeType());
    }

    void CopyUploadBuffer(const uint8* vb, const uint8* ib, size vb_size, size ib_size) {
        uint8* p;
        upload_buffer->Map(0, &Range(0, 0), (void**)&p);
        memcpy(p, vb, vb_size);
        memcpy(p + vb_size, ib, ib_size);
        upload_buffer->Unmap(0, nullptr);
    }

public:
    void Load(const Bundle& bundle, Context& context, CommandList& upload_list) {
        const auto asset = bundle.FindData(Id());
        const size vb_size = vertices.Stride() * vertex_count;
        const uint8* vb = asset.Mem() + vertices.Offset();
        const size ib_size = indices.Stride() * index_count;
        const uint8* ib = asset.Mem() + indices.Offset();
        CreateBuffers(context, vb_size, ib_size);
        CreateBufferViews(vb_size, ib_size);
        CopyUploadBuffer(vb, ib, vb_size, ib_size);

        upload_list.Native()->CopyBufferRegion(vertex_buffer.Get(), 0, upload_buffer.Get(), 0, vb_size);
        upload_list.Native()->CopyBufferRegion(index_buffer.Get(), 0, upload_buffer.Get(), vb_size, ib_size);

        Array<ResourceBarrier, 2> barriers;
        barriers.Add(ResourceBarrier::Transition(vertex_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
        barriers.Add(ResourceBarrier::Transition(index_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));
        upload_list.Native()->ResourceBarrier(barriers.UsedCount(), barriers.Values());
        upload_list.Native()->DiscardResource(upload_buffer.Get(), nullptr);
    }

    void SetAndDraw(CommandList& command_list, unsigned instance_count) const {
        command_list.Native()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        command_list.Native()->IASetVertexBuffers(0, 1, &vertex_buffer_view);
        command_list.Native()->IASetIndexBuffer(&index_buffer_view);
        command_list.Native()->DrawIndexedInstanced(index_count, instance_count, 0, 0, 0);
    }
};

class SurfaceDynamic : public Surface {
    Microsoft::WRL::ComPtr<ID3D12Resource> surface_buffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> upload_buffer;
    unsigned surface_index = 0;
    FixedArray<UINT, Context::BufferCount> render_target_index;

    void CreateBuffer(Context& context, const Surface& surface, unsigned depth_or_array_size, bool is_depth_stencil, bool is_render_target, unsigned mip_count, Vector4 clear_color) {
        const auto resource_desc = ResourceDesc::Tex2D(ResourceFormat(NativeFormat(surface.Format())), surface.Width(), surface.Height(), depth_or_array_size, mip_count, 1, 0, ResourceFlags(is_depth_stencil, is_render_target));
        const auto resource_state = ResourceStates(is_depth_stencil, is_render_target);
        ClearValue clear_value(NativeFormat(surface.Format()), &clear_color.x);
        Context::CheckResult(context.Device()->CreateCommittedResource(&HeapProperties(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &resource_desc, resource_state, is_render_target ? &clear_value : nullptr, IID_PPV_ARGS(&surface_buffer)));
        DEBUG_ONLY(Context::SetDebugName(surface_buffer.Get(), String("Surface ") + surface.Name() + String(" Buffer"));)
    }

    void CreateSRV(Context& context, Dimension dimension, PixelFormat pixel_format, bool is_cube_map, unsigned mip_count) {
        surface_index = context.SRVPool().Allocate();
        const auto srv_desc = ResourceViewDesc(dimension, pixel_format, is_cube_map, mip_count);
        context.Device()->CreateShaderResourceView(surface_buffer.Get(), &srv_desc, context.SRVPool().CPUHandle(surface_index));
    }

    void CreateRTV(Context& context) {
        for (UINT n = 0; n < Context::BufferCount; n++) {
            Microsoft::WRL::ComPtr<ID3D12Resource> buffer = surface_buffer;
            if (is_default)
                context.SwapChain()->GetBuffer(n, IID_PPV_ARGS(&buffer));
            render_target_index[n] = context.RTVPool().Allocate();
            const auto rtv_desc = RenderTargetViewDesc();
            context.Device()->CreateRenderTargetView(buffer.Get(), is_default ? nullptr : &rtv_desc, context.RTVPool().CPUHandle(render_target_index[n]));
        }
    }

    void CreateDSV(Context& context) {
        for (UINT n = 0; n < Context::BufferCount; n++) {
            Microsoft::WRL::ComPtr<ID3D12Resource> buffer = surface_buffer;
            render_target_index[n] = context.DSVPool().Allocate();
            const auto dsv_desc = DepthStencilViewDesc();
            context.Device()->CreateDepthStencilView(buffer.Get(), &dsv_desc, context.DSVPool().CPUHandle(render_target_index[n]));
        }
    }

    void DestroySRV(Context& context) {
        context.SRVPool().Free(surface_index);
    }

    void DestroyRTV(Context& context) {
        for (UINT n = 0; n < Context::BufferCount; n++)
            context.RTVPool().Free(render_target_index[n]);
    }

    void DestroyDSV(Context& context) {
        for (UINT n = 0; n < Context::BufferCount; n++)
            context.DSVPool().Free(render_target_index[n]);
    }

    Microsoft::WRL::ComPtr<ID3D12Resource> Buffer(Context& context) const {
        Microsoft::WRL::ComPtr<ID3D12Resource> buffer = surface_buffer;
        if (is_default)
            context.SwapChain()->GetBuffer(context.FrameIndex(), IID_PPV_ARGS(&buffer));
        return buffer;
    }

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

    void Recreate(Context& context) {
        if (!is_default) {
            CreateBuffer(context, *this, 1, is_depth_stencil, true, 1, clear_color);
            DestroySRV(context);
            CreateSRV(context, Dimension::Tex2D, pixel_format, false, 1);
        }
        if (is_depth_stencil) {
            DestroyDSV(context);
            CreateDSV(context);
        } else {
            DestroyRTV(context);
            CreateRTV(context);
        }
    }

    void CopyMips(const Bundle& bundle, Context& context, CommandList& upload_list) {
        UINT64 required_size = 0;
        D3D12_RESOURCE_DESC desc = surface_buffer.Get()->GetDesc();
        context.Device()->GetCopyableFootprints(&desc, 0, slice_count * mip_count, 0, nullptr, nullptr, nullptr, &required_size);
        Context::CheckResult(context.Device()->CreateCommittedResource(&HeapProperties(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &ResourceDesc::Buffer(required_size), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload_buffer)));
        DEBUG_ONLY(Context::SetDebugName(upload_buffer.Get(), String("Texture ") + Name() + String(" Upload Buffer"));)

        const auto asset = bundle.FindData(Id());
        const auto sub_resources = SubResources(asset.Mem());
        context.UpdateSubresources(upload_list.Native().Get(), surface_buffer.Get(), upload_buffer.Get(), 0, 0, slice_count * mip_count, sub_resources.Values());

        upload_list.Native()->ResourceBarrier(1, &ResourceBarrier::Transition(surface_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
        upload_list.Native()->DiscardResource(upload_buffer.Get(), nullptr);
    }

    FixedArray<D3D12_SUBRESOURCE_DATA, 64> SubResources(const uint8* data) {
        FixedArray<D3D12_SUBRESOURCE_DATA, 64> src_data;
        for (unsigned j = 0; j < slice_count; j++) {
            size w = width, h = height, d = depth;
            for (unsigned i = 0; i < mip_count; i++) {
                const unsigned index = mip_count * j + i;
                src_data[index].pData = data;
                ComputePitch(w, h, (size&)src_data[index].RowPitch, (size&)src_data[index].SlicePitch);
                data += src_data[index].SlicePitch * d;
                w = Math::Max(w >> 1, 1llu);
                h = Math::Max(h >> 1, 1llu);
                d = Math::Max(d >> 1, 1llu);
            }
        }
        return src_data;
    }

    void ComputePitch(size width, size height, size& row_pitch, size& slice_picth)
    {
        switch (NativeFormat(pixel_format))
        {
        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
            row_pitch = Math::Max(1llu, (width + 3) / 4) * 8;
            slice_picth = row_pitch * Math::Max(1llu, (height + 3) / 4);
            break;

        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC7_TYPELESS:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            row_pitch = Math::Max(1llu, (width + 3) / 4) * 16;
            slice_picth = row_pitch * Math::Max(1llu, (height + 3) / 4);
            break;

        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
            row_pitch = ((width + 1) >> 1) * 4;
            slice_picth = row_pitch * height;
            break;

        default:
            row_pitch = (width * bits_per_pixel + 7) / 8;
            slice_picth = row_pitch * height;
            break;
        }
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC ResourceViewDesc(Dimension dimension, PixelFormat pixel_format, bool is_cube_map, unsigned mip_count) {
        D3D12_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc;
        shader_resource_view_desc.Format = ResourceViewFormat(NativeFormat(pixel_format));
        shader_resource_view_desc.ViewDimension = ResourceViewDimension(dimension, is_cube_map);
        shader_resource_view_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        shader_resource_view_desc.Texture2D.PlaneSlice = 0;
        shader_resource_view_desc.Texture2D.MipLevels = mip_count;
        shader_resource_view_desc.Texture2D.MostDetailedMip = 0;
        shader_resource_view_desc.Texture2D.ResourceMinLODClamp = 0.f;
        return shader_resource_view_desc;
    }

    D3D12_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc() {
        D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {};
        rtv_desc.Format = NativeFormat(pixel_format);
        rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        return rtv_desc;
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc() {
        D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
        dsv_desc.Format = NativeFormat(pixel_format);
        dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsv_desc.Flags = D3D12_DSV_FLAG_NONE;
        return dsv_desc;
    }

    static D3D12_RESOURCE_FLAGS ResourceFlags(bool is_depth_stencil, bool is_render_target) {
        return is_depth_stencil ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL :
            is_render_target ? D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET :
            D3D12_RESOURCE_FLAG_NONE;
    }

    static D3D12_RESOURCE_STATES ResourceStates(bool is_depth_stencil, bool is_render_target) {
        return is_depth_stencil ? D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE :
            is_render_target ? D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE :
            D3D12_RESOURCE_STATE_COPY_DEST;
    }

    static D3D12_SRV_DIMENSION ResourceViewDimension(Dimension dimension, bool is_cube_map) {
        switch (dimension) {
        case Dimension::Tex1D: return D3D12_SRV_DIMENSION_TEXTURE1D;
        case Dimension::Tex2D: return is_cube_map ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;
        case Dimension::Tex3D: return D3D12_SRV_DIMENSION_TEXTURE3D;

        }
        return is_cube_map ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;
    }

    static DXGI_FORMAT ResourceFormat(DXGI_FORMAT native_format) { 
        switch (native_format) {
        case DXGI_FORMAT_D16_UNORM: return DXGI_FORMAT_R16_TYPELESS;
        case DXGI_FORMAT_D32_FLOAT: return DXGI_FORMAT_R32_TYPELESS;
        default: return native_format;
        };
    }
    static DXGI_FORMAT ResourceViewFormat(DXGI_FORMAT native_format) {
        switch (native_format) {
        case DXGI_FORMAT_D16_UNORM: return DXGI_FORMAT_R16_UNORM;
        case DXGI_FORMAT_D32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
        default: return native_format;
        };
    }

    static DXGI_FORMAT NativeFormat(PixelFormat pixel_format) {
        switch (pixel_format) {
        case PixelFormat::D16: return DXGI_FORMAT_D16_UNORM;
        case PixelFormat::D32F: return DXGI_FORMAT_D32_FLOAT;
        case PixelFormat::R8: return DXGI_FORMAT_R8_UNORM;
        case PixelFormat::R16F: return DXGI_FORMAT_R16_FLOAT;
        case PixelFormat::R32F: return DXGI_FORMAT_R32_FLOAT;
        case PixelFormat::RG8: return DXGI_FORMAT_R8G8_UNORM;
        case PixelFormat::RGB11F: return DXGI_FORMAT_R11G11B10_FLOAT;
        case PixelFormat::RGBA8: return DXGI_FORMAT_R8G8B8A8_UNORM;
        case PixelFormat::BGRA8: return DXGI_FORMAT_B8G8R8A8_UNORM;
        case PixelFormat::RGBA10: return DXGI_FORMAT_R10G10B10A2_UNORM;
        case PixelFormat::RGBA16F: return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case PixelFormat::RG16F: return DXGI_FORMAT_R16G16_FLOAT;
        case PixelFormat::BC1: return DXGI_FORMAT_BC1_UNORM;
        case PixelFormat::BC2: return DXGI_FORMAT_BC2_UNORM;
        case PixelFormat::BC3: return DXGI_FORMAT_BC3_UNORM;
        default: return DXGI_FORMAT_UNKNOWN;
        };
    }

    D3D12_RESOURCE_STATES ReadState() const { return is_depth_stencil ? D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE : is_default ? D3D12_RESOURCE_STATE_PRESENT : D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE; }
    D3D12_RESOURCE_STATES WriteState() const { return is_depth_stencil ? D3D12_RESOURCE_STATE_DEPTH_WRITE : D3D12_RESOURCE_STATE_RENDER_TARGET; }

public:
    void Load(const Bundle& bundle, Context& context, CommandList& upload_list) {
        if (is_texture) {
            CreateBuffer(context, *this, slice_count, false, false, mip_count, Vector4(0.f));
            CopyMips(bundle, context, upload_list);
        } else {
            if (!is_default) {
                CreateBuffer(context, *this, 1, is_depth_stencil, true, 1, clear_color);
            }
        }
    }

    void Create(Context& context) {
        if (is_texture) {
            CreateSRV(context, dimension, pixel_format, is_cube_map, mip_count);
        } else {
            if (!is_default) {
                CreateSRV(context, Dimension::Tex2D, pixel_format, false, 1);
            }
            if (is_depth_stencil)
                CreateDSV(context);
            else
                CreateRTV(context);
        }
    }

    void Destroy(Context& context) {
        if (is_texture) {
            DestroySRV(context);
        } else {
            if (!is_default) {
                DestroySRV(context);
            }
            if (is_depth_stencil)
                DestroyDSV(context);
            else
                DestroyRTV(context);
        }
    }

    static void Set(Context& context, CommandList& command_list, Array<SurfaceDynamic*, Camera::SurfaceMaxCount>& surfaces) {
        for (unsigned i = 0; i < surfaces.UsedCount(); ++i)
            command_list.Native()->SetGraphicsRootDescriptorTable(5 + i, context.SRVPool().GPUHandle(surfaces[i]->surface_index));
        for (unsigned i = surfaces.UsedCount(); i < Camera::SurfaceMaxCount; ++i) // NOTE: On a Resource Binding Tier 1 hardware, all descriptor tables declared in the currently set Root Signature must be populated, even if the shaders do not need the descriptor.
            command_list.Native()->SetGraphicsRootDescriptorTable(5 + i, context.SRVPool().GPUHandle(context.DefaultTextureIndex()));
    }

    void Resize(Context& context) {
        if (UpdateSize(context))
            Recreate(context);
    }

    void Clear(Context& context, CommandList& command_list) {
        if (do_clear) {
            if (is_depth_stencil) command_list.Native()->ClearDepthStencilView(context.DSVPool().CPUHandle(render_target_index[context.FrameIndex()]), D3D12_CLEAR_FLAG_DEPTH, clear_color.x, 0, 0, nullptr);
            else command_list.Native()->ClearRenderTargetView(context.RTVPool().CPUHandle(render_target_index[context.FrameIndex()]), &clear_color.x, 0, nullptr);
        }
    }

    void SetViewport(CommandList& command_list, const Vector2& dynamic_scale) {
        D3D12_VIEWPORT viewport = {};
        viewport.Width = (float)width * (is_default ? 1.f : dynamic_scale.x);
        viewport.Height = (float)height * (is_default ? 1.f : dynamic_scale.y);
        viewport.MaxDepth = 1.f;
        command_list.Native()->RSSetViewports(1, &viewport);
    }

    void SetScissor(CommandList& command_list, const Vector2& dynamic_scale) {
        D3D12_RECT scissor_rect = {};
        scissor_rect.right = static_cast<LONG>(width * (is_default ? 1.f : dynamic_scale.x));
        scissor_rect.bottom = static_cast<LONG>(height * (is_default ? 1.f : dynamic_scale.y));
        command_list.Native()->RSSetScissorRects(1, &scissor_rect);
    }

    Vector2 ScreenSize(const Vector2& dynamic_scale) const {
        return Vector2(width, height) * (is_default ? 1.f : dynamic_scale);
    }

    CPUDescriptorHandle CPUHandle(Context& context) const {
        return is_depth_stencil ? context.DSVPool().CPUHandle(render_target_index[context.FrameIndex()]) :
            context.RTVPool().CPUHandle(render_target_index[context.FrameIndex()]);
    }

    ResourceBarrier Push(Context& context) { return ResourceBarrier::Transition(Buffer(context).Get(), ReadState(), WriteState()); }
    ResourceBarrier Pop(Context& context) { return ResourceBarrier::Transition(Buffer(context).Get(), WriteState(), ReadState()); }
};

class Attachments : public NoCopy {
    Array<SurfaceDynamic*, Camera::ColorAttachmentMaxCount + 1> attachments;
    Context& context;
    CommandList& command_list;

    void Gather(const Array<SurfaceDynamic*, Camera::ColorAttachmentMaxCount>& color_attachments, SurfaceDynamic* depth_stencil_attachment) {
        color_attachments.ConstProcess([&](const auto& color_attachment) {
            attachments.Add(color_attachment);
        });
        if (depth_stencil_attachment) {
            attachments.Add(depth_stencil_attachment);
        }
    }

    void Resize(Context& context) {
        attachments.ConstProcess([&](const auto& attachment) {
            attachment->Resize(context);
        });
    }

    void Set(CommandList& command_list, const Array<SurfaceDynamic*, Camera::ColorAttachmentMaxCount>& color_attachments, SurfaceDynamic* depth_stencil_attachment) {
        Array<CPUDescriptorHandle, Camera::ColorAttachmentMaxCount> color_handles;
        CPUDescriptorHandle depth_stencil_handle = {};
        color_attachments.ConstProcess([&](const auto& color_attachment) {
            color_handles.Add(color_attachment->CPUHandle(context));
        });
        if (depth_stencil_attachment) {
            depth_stencil_handle = depth_stencil_attachment->CPUHandle(context);
        }
        command_list.Native()->OMSetRenderTargets(color_handles.UsedCount(), color_handles.Values(), FALSE, depth_stencil_attachment ? &depth_stencil_handle : nullptr);
    }

    void Clear(Context& context, CommandList& command_list, const Array<SurfaceDynamic*, Camera::ColorAttachmentMaxCount>& color_attachments, SurfaceDynamic* depth_stencil_attachment, bool clear_color, bool clear_depth) {
        if (clear_color) {
            color_attachments.ConstProcess([&](const auto& attachment) {
                attachment->Clear(context, command_list);
            });
        }
        if (depth_stencil_attachment && clear_depth) {
            depth_stencil_attachment->Clear(context, command_list);
        }
    }

    void SetViewport(Context& context, CommandList& command_list, const Array<SurfaceDynamic*, Camera::ColorAttachmentMaxCount>& color_attachments) {
        attachments[0]->SetViewport(command_list, context.DynamicScale());
        attachments[0]->SetScissor(command_list, context.DynamicScale());
    }

    void Push(CommandList& command_list) {
        Array<ResourceBarrier, Camera::ColorAttachmentMaxCount + 1> barriers;
        attachments.ConstProcess([&](const auto& attachment) {
            barriers.Add(attachment->Push(context));
        });
        command_list.Native()->ResourceBarrier(barriers.UsedCount(), barriers.Values());
    }

    void Pop(CommandList& command_list) {
        Array<ResourceBarrier, Camera::ColorAttachmentMaxCount + 1> barriers;
        attachments.ConstProcess([&](const auto& attachment) {
            barriers.Add(attachment->Pop(context));
        });
        command_list.Native()->ResourceBarrier(barriers.UsedCount(), barriers.Values());
    }

public:
    Attachments(Context& context, CommandList& command_list, const Array<SurfaceDynamic*, Camera::ColorAttachmentMaxCount>& color_attachments, SurfaceDynamic* depth_stencil_attachment, bool clear_color, bool clear_depth, bool last)
        : context(context), command_list(command_list) {
        Gather(color_attachments, depth_stencil_attachment);
        Resize(context);
        Push(command_list);
        Set(command_list, color_attachments, depth_stencil_attachment);
        Clear(context, command_list, color_attachments, depth_stencil_attachment, clear_color, clear_depth);
        SetViewport(context, command_list, color_attachments);
    }

    ~Attachments() {
        Pop(command_list);
    }

    Vector2 ScreenSize(const Vector2& dynamic_scale) const {
        return attachments[0]->ScreenSize(dynamic_scale); }
};

class ShaderDynamic : public Shader {
    static DXGI_FORMAT InputFormat(Input::Format format, unsigned& offset) {
        DXGI_FORMAT pixel_format = DXGI_FORMAT_UNKNOWN;
        switch (format) {
        case Input::Format::char4: pixel_format = DXGI_FORMAT_R8G8B8A8_SNORM; offset += 4; break;
        case Input::Format::uchar4: pixel_format = DXGI_FORMAT_R8G8B8A8_UNORM; offset += 4; break;
        case Input::Format::half2: pixel_format = DXGI_FORMAT_R16G16_FLOAT; offset += 4; break;
        case Input::Format::float3: pixel_format = DXGI_FORMAT_R32G32B32_FLOAT; offset += 12; break;
        default: break;
        }
        return pixel_format;
    }

    static char const* InputSemantic(Input::Semantic semantic) {
        switch (semantic) {
        case Input::Semantic::Position: return "POSITION";
        case Input::Semantic::Normal: return "NORMAL";
        case Input::Semantic::Texcoord: return "TEXCOORD";
        case Input::Semantic::Color: return "COLOR";
        default: return nullptr;
        }
    }

    static D3D12_FILTER SamplerFilter(Sampler::Filter filter) {
        switch (filter) {
        case Sampler::Filter::MinMagMipPoint: return D3D12_FILTER_MIN_MAG_MIP_POINT;
        case Sampler::Filter::MinMagMipLinear: return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        case Sampler::Filter::Anisotropic: return D3D12_FILTER_ANISOTROPIC;
        case Sampler::Filter::ComparisonMinMagMipLinear: return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        case Sampler::Filter::ComparisonAnisotropic: return D3D12_FILTER_COMPARISON_ANISOTROPIC;
        default: return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        }
    }

    static D3D12_TEXTURE_ADDRESS_MODE SamplerAddressMode(Sampler::AddressMode address_mode) {
        switch (address_mode) {
        case Sampler::AddressMode::Wrap: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        case Sampler::AddressMode::Mirror: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        case Sampler::AddressMode::Clamp: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        case Sampler::AddressMode::Border: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        case Sampler::AddressMode::MirrorOnce: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
        default: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }
    }

    static D3D12_COMPARISON_FUNC SamplerComparison(Sampler::Comparison comparison) {
        switch (comparison) {
        case Sampler::Comparison::Never: return D3D12_COMPARISON_FUNC_NEVER;
        case Sampler::Comparison::Less: return D3D12_COMPARISON_FUNC_LESS;
        case Sampler::Comparison::Equal: return D3D12_COMPARISON_FUNC_EQUAL;
        case Sampler::Comparison::LessEqual: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case Sampler::Comparison::Greater: return D3D12_COMPARISON_FUNC_GREATER;
        case Sampler::Comparison::NotEqual: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case Sampler::Comparison::GreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case Sampler::Comparison::Always: return D3D12_COMPARISON_FUNC_ALWAYS;
        default: return D3D12_COMPARISON_FUNC_ALWAYS;
        }
    }

    static D3D12_DESCRIPTOR_RANGE_TYPE DescriptorRangeType(Descriptor::Type type) {
        switch (type) {
        case Descriptor::Type::Texture: return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        case Descriptor::Type::Buffer: return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        default: return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        }
    }

public:
    void Load(const Bundle& bundle, Context& context) {
        const auto asset = bundle.FindData(Id());
        techniques.ProcessIndex([&](auto& technique, unsigned index) {
            techniques_dynamic[index].Load(context, *this, technique, asset.Mem());
        });
    }

    void Set(CommandList& command_list, const unsigned index) const {
        techniques_dynamic[index].Set(command_list);
    }

    static void SetBatchInstanceConstants(CommandList& command_list, uint64 gpu) {
        command_list.Native()->SetGraphicsRootConstantBufferView(0, gpu);
    }

    static void SetUniformsConstants(CommandList& command_list, const Vector4* uniforms) {
        command_list.Native()->SetGraphicsRoot32BitConstants(1, sizeof(Vector4) * Uniforms::UniformMaxCount / 4, uniforms, 0);
    }

    static void SetBatchCameraConstants(CommandList& command_list, uint64 gpu) {
        command_list.Native()->SetGraphicsRootConstantBufferView(2, gpu);
    }

    static void SetCameraConstants(CommandList& command_list, uint64 gpu) {
        command_list.Native()->SetGraphicsRootConstantBufferView(3, gpu);
    }

    static void SetMiscConstants(CommandList& command_list, uint64 gpu) {
        command_list.Native()->SetGraphicsRootConstantBufferView(4, gpu);
    }

private:
    
    class TechniqueDynamic : public NoCopy {
        Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_state;

    public:
        void Load(Context& context, const Shader& shader, const Technique& technique, const uint8* data) {
            CreateRootSignature(context, shader, technique);
            CreateGraphicsPipelineState(context, shader, technique, data);
        }

        void Set(CommandList& command_list) const {
            command_list.Native()->SetGraphicsRootSignature(root_signature.Get());
            command_list.Native()->SetPipelineState(pipeline_state.Get());
        }

    private:
        void CreateRootSignature(Context& context, const Shader& shader, const Technique& technique) {
            Microsoft::WRL::ComPtr<ID3DBlob> error;
            Microsoft::WRL::ComPtr<ID3DBlob> signature;
            const auto descriptor_ranges = DescriptorRanges(shader.descriptors);
            const auto static_sampler_descs = StaticSamplerDescs(shader.samplers);
            const auto root_parameters = GraphicsRootParameters(descriptor_ranges);
            const auto root_signature_flags = GraphicsRootSignatureFlags();
            RootSignatureDesc root_signature_desc(root_parameters.UsedCount(), root_parameters.Values(), static_sampler_descs.UsedCount(), static_sampler_descs.Values(), root_signature_flags);
            D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
            Context::CheckError(error.Get());
            Context::CheckResult(context.Device()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&root_signature)));
            DEBUG_ONLY(root_signature->SetName(L"Root Signature"));
        }

        void CreateGraphicsPipelineState(Context& context, const Shader& shader, const Technique& technique, const uint8* data) {
            const auto input_element_descs = InputElementDescs(shader.inputs);
            const auto pso_desc = GraphicsPipelineStateDesc(context, technique, input_element_descs, data);
            Context::CheckResult(context.Device()->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pipeline_state)));
            DEBUG_ONLY(Context::SetDebugName(pipeline_state.Get(), String("Shader ") + shader.Name() + String(" Graphics Pipeline State"));)
        }

        Array<DescriptorRange, Camera::SurfaceMaxCount> DescriptorRanges(const Array<Descriptor, DescriptorMaxCount>& descriptors) {
            Array<DescriptorRange, Camera::SurfaceMaxCount> descriptor_ranges;
            descriptors.ConstProcessIndex([&](const auto& descriptor, unsigned index) {
                descriptor_ranges.Add(DescriptorRange(DescriptorRangeType(descriptor.type), 1, index));
            });
            for (unsigned i = descriptor_ranges.UsedCount(); i < Camera::SurfaceMaxCount; ++i)
                descriptor_ranges.Add(DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, i));
            return descriptor_ranges;
        }

        Array<StaticSamplerDesc, 8> StaticSamplerDescs(const Array<Sampler, SamplerMaxCount>& samplers) {
            Array<StaticSamplerDesc, 8> static_sampler_descs;
            samplers.ConstProcessIndex([&](const auto& sampler, unsigned index) {
                static_sampler_descs.Add(index, SamplerFilter(sampler.filter), SamplerAddressMode(sampler.address_mode), SamplerAddressMode(sampler.address_mode), SamplerAddressMode(sampler.address_mode), 0.f, 16, SamplerComparison(sampler.comparison));
            });
            return static_sampler_descs;
        }

        Array<RootParameter, 16> GraphicsRootParameters(const Array<DescriptorRange, Camera::SurfaceMaxCount>& descriptor_ranges) {
            Array<RootParameter, 16> root_parameters;
            auto& root_parameter0 = root_parameters.Add(); root_parameter0.InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
            auto& root_parameter1 = root_parameters.Add(); root_parameter1.InitAsConstants(sizeof(Vector4) * Uniforms::UniformMaxCount / 4, 1, 0, D3D12_SHADER_VISIBILITY_ALL);
            auto& root_parameter2 = root_parameters.Add(); root_parameter2.InitAsConstantBufferView(2, 0, D3D12_SHADER_VISIBILITY_PIXEL);
            auto& root_parameter3 = root_parameters.Add(); root_parameter3.InitAsConstantBufferView(3, 0, D3D12_SHADER_VISIBILITY_ALL);
            auto& root_parameter4 = root_parameters.Add(); root_parameter4.InitAsConstantBufferView(4, 0, D3D12_SHADER_VISIBILITY_ALL);
            descriptor_ranges.ConstProcess([&](auto& descriptor_range) {
                auto& root_parameter = root_parameters.Add(); root_parameter.InitAsDescriptorTable(1, &descriptor_range, D3D12_SHADER_VISIBILITY_PIXEL);
            });
            return root_parameters;
        }

        static D3D12_ROOT_SIGNATURE_FLAGS ComputeRootSignatureFlags() {
            D3D12_ROOT_SIGNATURE_FLAGS root_signature_flags =
                D3D12_ROOT_SIGNATURE_FLAG_NONE;
            return root_signature_flags;
        }

        static D3D12_ROOT_SIGNATURE_FLAGS GraphicsRootSignatureFlags() {
            D3D12_ROOT_SIGNATURE_FLAGS root_signature_flags =
                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
            return root_signature_flags;
        }

        D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicsPipelineStateDesc(Context& context, const Technique& technique, const Array<D3D12_INPUT_ELEMENT_DESC, InputMaxCount>& input_element_descs, const uint8* data) {
            D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
            pso_desc.InputLayout = { input_element_descs.Values(), (UINT)input_element_descs.UsedCount() };
            pso_desc.pRootSignature = root_signature.Get();
            pso_desc.VS = ShaderBytecode(data + technique.vertex_binary.offset_from_base, technique.vertex_binary.size);
            pso_desc.PS = ShaderBytecode(data + technique.pixel_binary.offset_from_base, technique.pixel_binary.size);
            pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            pso_desc.SampleMask = UINT_MAX;
            pso_desc.SampleDesc.Count = 1;
            pso_desc.NumRenderTargets = technique.color_attachment_pixel_formats.UsedCount();
            for (unsigned i = 0; i < technique.color_attachment_pixel_formats.UsedCount(); ++i)
                pso_desc.RTVFormats[i] = ColorFormat(technique, i);
            pso_desc.DSVFormat = DepthFormat(technique);
            pso_desc.RasterizerState = RasterizerStateDesc(technique);
            pso_desc.BlendState = BlendStateDesc(technique);
            pso_desc.DepthStencilState = DepthStencilStateDesc(technique);
            return pso_desc;
        }

        Array<D3D12_INPUT_ELEMENT_DESC, InputMaxCount> InputElementDescs(const Array<Input, InputMaxCount>& inputs) {
            Array<D3D12_INPUT_ELEMENT_DESC, InputMaxCount> input_element_descs;
            UINT offset = 0;
            inputs.ConstProcess([&](const auto& input) {
                D3D12_INPUT_ELEMENT_DESC input_element_desc;
                input_element_desc.InputSlot = 0;
                input_element_desc.AlignedByteOffset = offset;
                input_element_desc.SemanticName = InputSemantic(input.semantic);
                input_element_desc.SemanticIndex = input.index;
                input_element_desc.Format = InputFormat(input.format, offset);
                input_element_desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                input_element_desc.InstanceDataStepRate = 0;
                input_element_descs.Add(input_element_desc);
            });
            return input_element_descs;
        }

        D3D12_BLEND_DESC BlendStateDesc(const Technique& technique) {
            D3D12_BLEND_DESC blend_state = BlendDesc();
            for (unsigned i = 0; i < technique.color_attachment_pixel_formats.UsedCount(); ++i)
                blend_state.RenderTarget[i] = RenderTargetBlendStateDesc(technique);
            return blend_state;
        }

        D3D12_RENDER_TARGET_BLEND_DESC RenderTargetBlendStateDesc(const Technique& technique) {
            D3D12_RENDER_TARGET_BLEND_DESC render_target_blend_desc = RenderTargetBlendDesc();
            render_target_blend_desc.BlendEnable = (technique.state_flags & State::Alpha) ? TRUE : FALSE;
            render_target_blend_desc.SrcBlend = SrcBlend(technique);
            render_target_blend_desc.DestBlend = DstBlend(technique);
            render_target_blend_desc.BlendOp = D3D12_BLEND_OP_ADD;
            render_target_blend_desc.SrcBlendAlpha = SrcBlend(technique);
            render_target_blend_desc.DestBlendAlpha = DstBlend(technique);
            render_target_blend_desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
            return render_target_blend_desc;
        }

        D3D12_RASTERIZER_DESC RasterizerStateDesc(const Technique& technique) {
            D3D12_RASTERIZER_DESC rasterizer_desc = RasterizerDesc();
            rasterizer_desc.FrontCounterClockwise = FALSE;
            rasterizer_desc.CullMode = CullMode(technique);
            rasterizer_desc.DepthBias = (UINT)(technique.depth_bias * Math::Pow(2.f, 16.0f));
            rasterizer_desc.DepthBiasClamp = 0.001f;
            rasterizer_desc.SlopeScaledDepthBias = technique.slope_scale_depth_bias;
            return rasterizer_desc;
        }

        D3D12_DEPTH_STENCIL_DESC DepthStencilStateDesc(const Technique& technique) {
            D3D12_DEPTH_STENCIL_DESC depth_stencil_desc = DepthStencilDesc();
            depth_stencil_desc.DepthEnable = (DepthFormat(technique) != DXGI_FORMAT_UNKNOWN);
            depth_stencil_desc.DepthFunc = (technique.state_flags & State::DepthTest) ? D3D12_COMPARISON_FUNC_LESS : D3D12_COMPARISON_FUNC_ALWAYS;
            depth_stencil_desc.DepthWriteMask = (technique.state_flags & State::DepthWrite) ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
            depth_stencil_desc.StencilEnable = FALSE;
            return depth_stencil_desc;
        }

        D3D12_CULL_MODE CullMode(const Technique& technique) {
            D3D12_CULL_MODE cull_mode = D3D12_CULL_MODE_NONE;
            if (technique.state_flags & State::FrontFaceCull) { cull_mode = D3D12_CULL_MODE_FRONT; }
            else if (technique.state_flags & State::BackFaceCull) { cull_mode = D3D12_CULL_MODE_BACK; }
            return cull_mode;
        }

        D3D12_BLEND SrcBlend(const Technique& technique) {
            D3D12_BLEND src = D3D12_BLEND_ZERO;
            if (technique.state_flags & State::AlphaPreMultiplied) { src = D3D12_BLEND_ONE; }
            else if (technique.state_flags & State::AlphaPostMultiplied) { src = D3D12_BLEND_SRC_ALPHA; }
            else if (technique.state_flags & State::AlphaAdditive) { src = D3D12_BLEND_SRC_ALPHA; }
            else if (technique.state_flags & State::AlphaMultiplicative) { src = D3D12_BLEND_DEST_COLOR; }
            return src;
        }

        D3D12_BLEND DstBlend(const Technique& technique) {
            D3D12_BLEND dst = D3D12_BLEND_ZERO;
            if (technique.state_flags & State::AlphaPreMultiplied) { dst = D3D12_BLEND_INV_SRC_ALPHA; }
            else if (technique.state_flags & State::AlphaPostMultiplied) { dst = D3D12_BLEND_INV_SRC_ALPHA; }
            else if (technique.state_flags & State::AlphaAdditive) { dst = D3D12_BLEND_ONE; }
            else if (technique.state_flags & State::AlphaMultiplicative) { dst = D3D12_BLEND_ZERO; }
            return dst;
        }

        DXGI_FORMAT ColorFormat(const Technique& technique, unsigned index) {
            switch (technique.color_attachment_pixel_formats[index]) {
            case PixelFormat::R16F: return DXGI_FORMAT_R16_FLOAT;
            case PixelFormat::R32F: return DXGI_FORMAT_R32_FLOAT;
            case PixelFormat::RGB11F: return DXGI_FORMAT_R11G11B10_FLOAT;
            case PixelFormat::BGRA8: return DXGI_FORMAT_B8G8R8A8_UNORM;
            case PixelFormat::RGBA8: return DXGI_FORMAT_R8G8B8A8_UNORM;
            case PixelFormat::RGBA10: return DXGI_FORMAT_R10G10B10A2_UNORM;
            case PixelFormat::RGBA16F: return DXGI_FORMAT_R16G16B16A16_FLOAT;
            default: return DXGI_FORMAT_UNKNOWN;
            }
        }

        DXGI_FORMAT DepthFormat(const Technique& technique) {
            switch (technique.depth_attachment_pixel_format) {
            case PixelFormat::D16: return DXGI_FORMAT_D16_UNORM;
            case PixelFormat::D32F: return DXGI_FORMAT_D32_FLOAT;
            default: return DXGI_FORMAT_UNKNOWN;
            }
        }
    };

    FixedArray<TechniqueDynamic, TechniqueMaxCount> techniques_dynamic;
};

class Timings : public NoCopy {
public:
    static const unsigned BufferCount = Context::BufferCount + 1;

private:
    static const Color TimingsColor = Color::Aqua;
    static const unsigned TimingsMaxCount = 64;

    FixedArray<Microsoft::WRL::ComPtr<ID3D12QueryHeap>, BufferCount> query_heap;
    FixedArray<Microsoft::WRL::ComPtr<ID3D12Resource>, BufferCount> read_back_buffer;
    FixedArray<uint64, BufferCount> gpu_start;
    FixedArray<uint64, BufferCount> cpu_start;
    FixedArray<UINT, BufferCount> gather_count;
    FixedArray<UINT, BufferCount> push_count;
    uint64 gpu_frequency = 0;
    uint64* gpu_timestamps = nullptr;
    uint64 frame_total = 0;
    unsigned gather_index = 0;
    unsigned push_index = 0;

    D3D12_HEAP_PROPERTIES HeapProperties() {
        D3D12_HEAP_PROPERTIES heap_props;
        heap_props.Type = D3D12_HEAP_TYPE_READBACK;
        heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heap_props.CreationNodeMask = 1;
        heap_props.VisibleNodeMask = 1;
        return heap_props;
    }

    D3D12_RESOURCE_DESC BufferDesc() {
        D3D12_RESOURCE_DESC buffer_desc;
        buffer_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        buffer_desc.Alignment = 0;
        buffer_desc.Width = sizeof(uint64) * TimingsMaxCount;
        buffer_desc.Height = 1;
        buffer_desc.DepthOrArraySize = 1;
        buffer_desc.MipLevels = 1;
        buffer_desc.Format = DXGI_FORMAT_UNKNOWN;
        buffer_desc.SampleDesc.Count = 1;
        buffer_desc.SampleDesc.Quality = 0;
        buffer_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        buffer_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        return buffer_desc;
    }

    D3D12_QUERY_HEAP_DESC QueryHeapDesc() {
        D3D12_QUERY_HEAP_DESC query_heap_desc;
        query_heap_desc.Count = TimingsMaxCount;
        query_heap_desc.NodeMask = 1;
        query_heap_desc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
        return query_heap_desc;
    }

public:
    Timings() {}
    Timings(Context& context) : gather_index(1) {
        for (unsigned i = 0; i < BufferCount; ++i) {
            query_heap[i] = nullptr;
            read_back_buffer[i] = nullptr;
            gather_count[i] = 0;
            push_count[i] = 1;

            const auto heap_props_desc = HeapProperties();
            const auto buffer_desc = BufferDesc();
            Context::CheckResult(context.Device()->CreateCommittedResource(&heap_props_desc, D3D12_HEAP_FLAG_NONE, &buffer_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&read_back_buffer[i])));
            DEBUG_ONLY(read_back_buffer[i]->SetName(L"Timings Read Back Buffer"));

            const auto query_heap_desc = QueryHeapDesc();
            Context::CheckResult(context.Device()->CreateQueryHeap(&query_heap_desc, IID_PPV_ARGS(&query_heap[i])));
            DEBUG_ONLY(query_heap[i]->SetName(L"Timings Query Heap"));
        }
    }

    void Swap(Context& context, uint64 now) {
        gather_index = (gather_index + 1) % BufferCount;
        push_index = (push_index + 1) % BufferCount;

        uint64 gpu_timestamp, cpu_timestamp;
        Context::CheckResult(context.CommandQueue()->GetClockCalibration(&gpu_timestamp, &cpu_timestamp));
        Context::CheckResult(context.CommandQueue()->GetTimestampFrequency(&gpu_frequency));

        gpu_start[push_index] = gpu_timestamp;
        cpu_start[push_index] = now;
    }

    void Push(CommandList& command_list) {
        command_list.Native()->EndQuery(query_heap[push_index].Get(), D3D12_QUERY_TYPE_TIMESTAMP, push_count[push_index]++);
    }

    void Query(CommandList& command_list) {
        command_list.Native()->ResolveQueryData(query_heap[push_index].Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, push_count[push_index], read_back_buffer[push_index].Get(), 0);
    }

    void GatherBegin() {
        D3D12_RANGE range;
        range.Begin = 0;
        range.End = push_count[gather_index] * sizeof(uint64);
        Context::CheckResult(read_back_buffer[gather_index]->Map(0, &range, (void**)(&gpu_timestamps)));
        frame_total = 0;
    }

    void GatherOne(Profile& profile, Color color) {
        const uint64 gpu = gpu_start[gather_index];
        const uint64 cpu = cpu_start[gather_index];
        const unsigned index = gather_count[gather_index];
        const uint64 gpu_begin = gpu_timestamps[index];
        const uint64 gpu_end = gpu_timestamps[index + 1];
        if ((gpu_begin != 0) && (gpu_end != 0)) {
            const uint64 cpu_begin = cpu + (gpu_begin - gpu) * 1000000 / gpu_frequency;
            const uint64 cpu_end = cpu + (gpu_end - gpu) * 1000000 / gpu_frequency;
            DEBUG_ONLY(profile.BeginEndGPU(cpu_begin, cpu_end, color);)
            frame_total += cpu_end - cpu_begin;
        }
        gather_count[gather_index] += 2;
    }

    void GatherEnd() {
        D3D12_RANGE empty_range = {};
        read_back_buffer[gather_index]->Unmap(0, &empty_range);
        gather_count[gather_index] = 0;
        push_count[gather_index] = 0;
    }

    uint64 Total() const { return frame_total; }
};
