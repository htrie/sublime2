
class DebugDraw : public NoCopy {
    static const size BufferSize = 16 * 1024 * 1024;

#pragma pack(push, 1)
    struct Uniforms {
        float viewproj[16];
    };

    struct Vertex {
        float position[3];
        unsigned char color[4];
    };
#pragma pack(pop)

    Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_state_triangle;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_state_line;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> transient_command_list;
    Microsoft::WRL::ComPtr<ID3D12Resource> font_texture;
    Microsoft::WRL::ComPtr<ID3D12Resource> font_texture_upload;
    FixedArray<Microsoft::WRL::ComPtr<ID3D12Resource>, Context::BufferCount> buffers;
    FixedArray<UINT8*, Context::BufferCount> buffer;
    size buffer_offset = 0;
    UINT buffer_index = 0;

    static const char debug_shader[];

    void InitBuffers(Context& context) {
        for (unsigned i = 0; i < Context::BufferCount; ++i) {
            Context::CheckResult(context.Device()->CreateCommittedResource(&HeapProperties(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &ResourceDesc::Buffer(BufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&(buffers[i]))));
            DEBUG_ONLY(buffers[i]->SetName(L"Debug Draw Heap"));
            buffers[i]->Map(0, &Range(0, 0), (void**)(&(buffer[i])));
        }
    }

    void InitRootSignature(Context& context) {
        const auto root_parameters = RootParameters();
        const auto root_signature_flags = RootSignatureFlags();
        RootSignatureDesc root_signature_desc(root_parameters.Count(), root_parameters.Values(), 0, NULL, root_signature_flags);
        Microsoft::WRL::ComPtr<ID3DBlob> error;
        Microsoft::WRL::ComPtr<ID3DBlob> signature;
        D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
        Context::CheckError(error.Get());

        Context::CheckResult(context.Device()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&root_signature)));
        DEBUG_ONLY(root_signature->SetName(L"Debug Draw Root Signature"));
    }

    void InitPipelineStates(Context& context) {
        const auto input_element_descs = InputElementDescs();
        auto vertex_shader = CompileShader("vertex_main", "vs_5_0");
        auto pixel_shader = CompileShader("fragment_main", "ps_5_0");

        const auto line_pso_desc = PipelineStateDesc(input_element_descs, vertex_shader.Get(), pixel_shader.Get(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
        Context::CheckResult(context.Device()->CreateGraphicsPipelineState(&line_pso_desc, IID_PPV_ARGS(&pipeline_state_line)));
        DEBUG_ONLY(pipeline_state_line->SetName(L"Debug Draw Line Pipeline State"));

        const auto triangle_pso_desc = PipelineStateDesc(input_element_descs, vertex_shader.Get(), pixel_shader.Get(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
        Context::CheckResult(context.Device()->CreateGraphicsPipelineState(&triangle_pso_desc, IID_PPV_ARGS(&pipeline_state_triangle)));
        DEBUG_ONLY(pipeline_state_triangle->SetName(L"Debug Draw Triangle Pipeline State"));
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineStateDesc(const FixedArray<D3D12_INPUT_ELEMENT_DESC, 2>& input_element_descs, ID3DBlob* vertex_shader, ID3DBlob* pixel_shader, D3D12_PRIMITIVE_TOPOLOGY_TYPE primitive_type) const {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
        pso_desc.InputLayout = { input_element_descs.Values(), (UINT)input_element_descs.Count() };
        pso_desc.pRootSignature = root_signature.Get();
        pso_desc.VS = ShaderBytecode(vertex_shader);
        pso_desc.PS = ShaderBytecode(pixel_shader);
        pso_desc.PrimitiveTopologyType = primitive_type;
        pso_desc.SampleMask = UINT_MAX;
        pso_desc.SampleDesc.Count = 1;
        pso_desc.NumRenderTargets = 1;
        pso_desc.RTVFormats[0] = DXGI_FORMAT_R10G10B10A2_UNORM;
        pso_desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
        pso_desc.RasterizerState = RasterizerStateDesc();
        pso_desc.BlendState = BlendStateDesc();
        pso_desc.DepthStencilState = DepthStencilStateDesc();
        return pso_desc;
    }

    static D3D12_BLEND_DESC BlendStateDesc() {
        D3D12_BLEND_DESC blend_state = BlendDesc();
        blend_state.RenderTarget[0] = RenderTargetBlendStateDesc();
        return blend_state;
    }

    static D3D12_RENDER_TARGET_BLEND_DESC RenderTargetBlendStateDesc() {
        D3D12_RENDER_TARGET_BLEND_DESC render_target_blend_desc = RenderTargetBlendDesc();
        render_target_blend_desc.BlendEnable = TRUE;
        render_target_blend_desc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
        render_target_blend_desc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        render_target_blend_desc.BlendOp = D3D12_BLEND_OP_ADD;
        render_target_blend_desc.SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
        render_target_blend_desc.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
        render_target_blend_desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
        return render_target_blend_desc;
    }

    static D3D12_RASTERIZER_DESC RasterizerStateDesc() {
        D3D12_RASTERIZER_DESC rasterizer_desc = RasterizerDesc();
        rasterizer_desc.FrontCounterClockwise = FALSE;
        rasterizer_desc.CullMode = D3D12_CULL_MODE_NONE;
        return rasterizer_desc;
    }

    static D3D12_DEPTH_STENCIL_DESC DepthStencilStateDesc() {
        D3D12_DEPTH_STENCIL_DESC depth_stencil_desc = DepthStencilDesc();
        depth_stencil_desc.DepthEnable = FALSE;
        depth_stencil_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        depth_stencil_desc.StencilEnable = FALSE;
        return depth_stencil_desc;
    }

    static FixedArray<RootParameter, 1> RootParameters() {
        FixedArray<RootParameter, 1> root_parameters;
        root_parameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
        return root_parameters;
    }

    static D3D12_ROOT_SIGNATURE_FLAGS RootSignatureFlags() {
        D3D12_ROOT_SIGNATURE_FLAGS root_signature_flags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
        return root_signature_flags;
    }

    static FixedArray<D3D12_INPUT_ELEMENT_DESC, 2> InputElementDescs() {
        FixedArray<D3D12_INPUT_ELEMENT_DESC, 2> input_element_descs;
        input_element_descs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
        input_element_descs[1] = { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
        return input_element_descs;
    }

    static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const char* entry_point, const char* target) {
        UINT compile_flags = 0;
        Microsoft::WRL::ComPtr<ID3DBlob> error;
        Microsoft::WRL::ComPtr<ID3DBlob> shader;
        D3DCompile(debug_shader, Math::Length(debug_shader), "DebugShader", nullptr, nullptr, entry_point, target, compile_flags, 0, &shader, &error);
        Context::CheckError(error.Get());
        return shader;
    }

public:
    DebugDraw() {}
    DebugDraw(Context& context) {
        InitRootSignature(context);
        InitPipelineStates(context);
        InitBuffers(context);
    }

    size BufferMemory() const { return BufferSize * Context::BufferCount; }

    void Swap() {
        buffer_index = (buffer_index + 1) % Context::BufferCount;
        buffer_offset = 0;
    }

    void Reset(CommandList& command_list) {
        transient_command_list = command_list.Native();
    }

    void Begin(Context& context) {
        transient_command_list->SetGraphicsRootSignature(root_signature.Get());
    }

    size AlignBufferOffset() {
        const size buffer_size = Math::AlignSize(buffer_offset, (size)D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        buffer_offset = Math::Min(buffer_size, BufferSize);
        return buffer_offset;
    }

    size PushData(size data_size, const uint8* data) {
        if ((buffer_offset + data_size) > BufferSize)
            return 0;

        const size offset = buffer_offset;
        memcpy(buffer[buffer_index] + buffer_offset, data, data_size);
        buffer_offset += data_size;
        return offset;
    }

    void PushVertex(const Vector3& position, uint32 color, unsigned& vertex_count) {
        Vertex v;
        *(Vector3*)&v.position = position;
        *(uint32*)&v.color = color;
        if (PushData(sizeof(Vertex), (uint8*)&v))
            vertex_count++;
    }

    void PushLine(const Vector3& v0, const Vector3& v1, uint32 color, unsigned& vertex_count) {
        PushVertex(v0, color, vertex_count);
        PushVertex(v1, color, vertex_count);
    }

    void PushTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, uint32 color, unsigned& vertex_count) {
        PushVertex(v0, color, vertex_count);
        PushVertex(v1, color, vertex_count);
        PushVertex(v2, color, vertex_count);
    }

    void SetConstantBuffer(size uniform_buffer_offset) {
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc;
        cbv_desc.BufferLocation = buffers[buffer_index]->GetGPUVirtualAddress() + uniform_buffer_offset;
        cbv_desc.SizeInBytes = (UINT)Math::AlignSize(sizeof(Uniforms), (size)D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        transient_command_list->SetGraphicsRootConstantBufferView(0, cbv_desc.BufferLocation);
    }

    void SetVertexBuffer(size data_buffer_offset, unsigned vertex_count) {
        D3D12_VERTEX_BUFFER_VIEW vbv;
        vbv.BufferLocation = buffers[buffer_index]->GetGPUVirtualAddress() + data_buffer_offset;
        vbv.StrideInBytes = (UINT)sizeof(Vertex);
        vbv.SizeInBytes = (UINT)(sizeof(Vertex) * vertex_count);
        transient_command_list->IASetVertexBuffers(0, 1, &vbv);
    }

    void DrawPrimitives(unsigned vertex_count, bool is_triangle) {
        transient_command_list->SetPipelineState(is_triangle ? pipeline_state_triangle.Get() : pipeline_state_line.Get());
        transient_command_list->IASetPrimitiveTopology(is_triangle ? D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST : D3D_PRIMITIVE_TOPOLOGY_LINELIST);
        transient_command_list->DrawInstanced(vertex_count, 1, 0, 0);
    }
};

const char DebugDraw::debug_shader[] =
    "cbuffer SceneConstantBuffer : register(b0)\n"
    "{\n"
    "   float4x4 viewproj;\n"
    "};\n"
    "struct VSInput\n"
    "{\n"
    "   float4 position : POSITION;\n"
    "   float4 color : COLOR;\n"
    "};\n"
    "struct PSInput\n"
    "{\n"
    "   float4 position : SV_POSITION;\n"
    "   float4 color : COLOR;\n"
    "};\n"
    "PSInput vertex_main(VSInput input)\n"
    "{\n"
    "   PSInput result;\n"
    "   result.position = mul(viewproj, input.position);\n"
    "   result.color = input.color;\n"
    "   return result;\n"
    "}\n"
    "float4 fragment_main(PSInput input) : SV_TARGET\n"
    "{\n"
    "   return input.color;\n"
    "}\n";
