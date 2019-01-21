
class DebugDraw : public NoCopy {
    static const size BufferSize = 16 * 1024 * 1024;
    
#pragma pack(push, 1)
    struct Uniforms {
        float viewproj[16];
    };
    
    struct Vertex {
        float position[3];
        float texcoords[2];
        unsigned char color[4];
    };
#pragma pack(pop)
    
    static const char debug_shader[];

    id <MTLRenderCommandEncoder> transient_command_encoder;

    id <MTLBuffer> _dynamicUniformBuffer[MaxBuffersInFlight];
    uint8_t _uniformBufferIndex;

    id <MTLRenderPipelineState> _pipelineState;
    id <MTLDepthStencilState> _depthState;
    id <MTLTexture> _colorMap;
    MTLVertexDescriptor *_mtlVertexDescriptor;
    MTKMesh* _mesh;

public:
    DebugDraw() {}
    DebugDraw(Context& context) {
        for(NSUInteger i = 0; i < MaxBuffersInFlight; i++)
        {
            _dynamicUniformBuffer[i] = [context.Device() newBufferWithLength:sizeof(Uniforms) options:MTLResourceStorageModeShared];
            _dynamicUniformBuffer[i].label = @"UniformBuffer";
        }

        _mtlVertexDescriptor = [[MTLVertexDescriptor alloc] init];

        _mtlVertexDescriptor.attributes[0].format = MTLVertexFormatFloat3;
        _mtlVertexDescriptor.attributes[0].offset = 0;
        _mtlVertexDescriptor.attributes[0].bufferIndex = 0;

        _mtlVertexDescriptor.attributes[1].format = MTLVertexFormatFloat2;
        _mtlVertexDescriptor.attributes[1].offset = 0;
        _mtlVertexDescriptor.attributes[1].bufferIndex = 1;

        _mtlVertexDescriptor.layouts[0].stride = 12;
        _mtlVertexDescriptor.layouts[0].stepRate = 1;
        _mtlVertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

        _mtlVertexDescriptor.layouts[1].stride = 8;
        _mtlVertexDescriptor.layouts[1].stepRate = 1;
        _mtlVertexDescriptor.layouts[1].stepFunction = MTLVertexStepFunctionPerVertex;

        auto source = [NSString stringWithUTF8String:debug_shader];
        NSError* error = nil;
        id<MTLLibrary> library = [context.Device() newLibraryWithSource:source options:nil error:&error];
        if(!library) {
            [NSException raise:@"Failed to compile shaders" format:@"%@", [error localizedDescription]];
        }

        id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertexShader"];
        id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragmentShader"];

        MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineStateDescriptor.label = @"MyPipeline";
        pipelineStateDescriptor.sampleCount = context.View().sampleCount;
        pipelineStateDescriptor.vertexFunction = vertexFunction;
        pipelineStateDescriptor.fragmentFunction = fragmentFunction;
        pipelineStateDescriptor.vertexDescriptor = _mtlVertexDescriptor;
        pipelineStateDescriptor.colorAttachments[0].pixelFormat = context.View().colorPixelFormat;
        pipelineStateDescriptor.depthAttachmentPixelFormat = context.View().depthStencilPixelFormat;
        pipelineStateDescriptor.stencilAttachmentPixelFormat = context.View().depthStencilPixelFormat;

        _pipelineState = [context.Device() newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
        DEBUG_ONLY(if (!_pipelineState) throw Exception("newRenderPipelineStateWithDescriptor" );)

        MTLDepthStencilDescriptor *depthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
        depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
        depthStateDesc.depthWriteEnabled = YES;
        _depthState = [context.Device() newDepthStencilStateWithDescriptor:depthStateDesc];
        MTKMeshBufferAllocator *metalAllocator = [[MTKMeshBufferAllocator alloc] initWithDevice: context.Device()];

        MDLMesh *mdlMesh = [MDLMesh newBoxWithDimensions:(vector_float3){4, 4, 4}
                                                segments:(vector_uint3){2, 2, 2}
                                            geometryType:MDLGeometryTypeTriangles
                                           inwardNormals:NO
                                               allocator:metalAllocator];

        MDLVertexDescriptor *mdlVertexDescriptor =
        MTKModelIOVertexDescriptorFromMetal(_mtlVertexDescriptor);

        mdlVertexDescriptor.attributes[0].name  = MDLVertexAttributePosition;
        mdlVertexDescriptor.attributes[1].name  = MDLVertexAttributeTextureCoordinate;

        mdlMesh.vertexDescriptor = mdlVertexDescriptor;

        _mesh = [[MTKMesh alloc] initWithMesh:mdlMesh device:context.Device() error:&error];
        DEBUG_ONLY(if (!_mesh || error) throw Exception("initWithMesh");)

        MTKTextureLoader* textureLoader = [[MTKTextureLoader alloc] initWithDevice:context.Device()];

        NSDictionary *textureLoaderOptions =
        @{
          MTKTextureLoaderOptionTextureUsage       : @(MTLTextureUsageShaderRead),
          MTKTextureLoaderOptionTextureStorageMode : @(MTLStorageModePrivate)
          };

        _colorMap = [textureLoader newTextureWithName:@"ColorMap" scaleFactor:1.0 bundle:nil options:textureLoaderOptions error:&error];
        DEBUG_ONLY(if (!_colorMap || error) throw Exception("newTextureWithName");)
    }
    
    size BufferMemory() const { return BufferSize * Context::BufferCount; }
    
    void Swap() {
    }
    
    void Reset(CommandList& command_list) {
        transient_command_encoder = command_list.Encoder();
    }
    
    void Begin(Context& context) {
        float aspect = (float)context.WindowWidth() / (float)context.WindowHeight();
        const auto modelMatrix = Matrix();
        const auto viewMatrix = Matrix(Vector3(0.f, 0.f, -8.f));
        const auto modelViewMatrix = viewMatrix * modelMatrix;
        Matrix projectionMatrix, projectionMatrixInverse;
        Matrix::PerspectiveFovRH(65.0f, aspect, 0.1f, 100.0f, projectionMatrix, projectionMatrixInverse);
        Matrix modelviewproj = modelViewMatrix * projectionMatrix;

        _uniformBufferIndex = (_uniformBufferIndex + 1) % MaxBuffersInFlight;
        Uniforms* uniforms = (Uniforms*)_dynamicUniformBuffer[_uniformBufferIndex].contents;
        memcpy(uniforms, &modelviewproj, sizeof(Matrix));

        [transient_command_encoder pushDebugGroup:@"DrawBox"];

        [transient_command_encoder setFrontFacingWinding:MTLWindingCounterClockwise];
        [transient_command_encoder setCullMode:MTLCullModeBack];
        [transient_command_encoder setRenderPipelineState:_pipelineState];
        [transient_command_encoder setDepthStencilState:_depthState];

        [transient_command_encoder setVertexBuffer:_dynamicUniformBuffer[_uniformBufferIndex]
                                offset:0
                               atIndex:2];

        [transient_command_encoder setFragmentBuffer:_dynamicUniformBuffer[_uniformBufferIndex]
                                  offset:0
                                 atIndex:2];

        for (NSUInteger bufferIndex = 0; bufferIndex < _mesh.vertexBuffers.count; bufferIndex++)
        {
            MTKMeshBuffer *vertexBuffer = _mesh.vertexBuffers[bufferIndex];
            if((NSNull*)vertexBuffer != [NSNull null])
            {
                [transient_command_encoder setVertexBuffer:vertexBuffer.buffer
                                        offset:vertexBuffer.offset
                                       atIndex:bufferIndex];
            }
        }

        [transient_command_encoder setFragmentTexture:_colorMap atIndex:0];

        for(MTKSubmesh *submesh in _mesh.submeshes)
        {
            [transient_command_encoder drawIndexedPrimitives:submesh.primitiveType
                                      indexCount:submesh.indexCount
                                       indexType:submesh.indexType
                                     indexBuffer:submesh.indexBuffer.buffer
                               indexBufferOffset:submesh.indexBuffer.offset];
        }

        [transient_command_encoder popDebugGroup];
    }
    
    size AlignBufferOffset() {
        return 0;
    }
    
    size PushData(size data_size, const uint8* data) {
        return 0;
    }
    
    void PushVertex(const Vector3& position, const Vector2& texcoords, uint32 color, unsigned& vertex_count) {
    }
    
    void PushLine(const Vector3& v0, const Vector3& v1, uint32 color, unsigned& vertex_count) {
        PushVertex(v0, Vector2(), color, vertex_count);
        PushVertex(v1, Vector2(), color, vertex_count);
    }
    
    void PushTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, uint32 color, unsigned& vertex_count) {
        PushVertex(v0, Vector2(), color, vertex_count);
        PushVertex(v1, Vector2(), color, vertex_count);
        PushVertex(v2, Vector2(), color, vertex_count);
    }
    
    void SetConstantBuffer(size uniform_buffer_offset) {
    }
    
    void SetVertexBuffer(size data_buffer_offset, unsigned vertex_count) {
    }
    
    void SetIndexBuffer(size index_buffer_offset, unsigned int index_count) {
    }
    
    void DrawIndexedPrimitives(unsigned index_count) {
    }
    
    void DrawPrimitives(unsigned vertex_count, bool is_triangle) {
    }
};

const char DebugDraw::debug_shader[] =
"#include <metal_stdlib>\n"
"#include <simd/simd.h>\n"
"\n"
"using namespace metal;\n"
"\n"
"typedef struct {\n"
"    matrix_float4x4 modelViewProjMatrix;\n"
"} Uniforms;\n"
"\n"
"typedef struct {\n"
"    float3 position [[attribute(0)]];\n"
"    float2 texCoord [[attribute(1)]];\n"
"} Vertex;\n"
"\n"
"typedef struct {\n"
"    float4 position [[position]];\n"
"    float2 texCoord;\n"
"} ColorInOut;\n"
"\n"
"vertex ColorInOut vertexShader(Vertex in [[stage_in]],\n"
"                               constant Uniforms & uniforms [[ buffer(2) ]]) {\n"
"    ColorInOut out;\n"
"    float4 position = float4(in.position, 1.0);\n"
"    out.position = uniforms.modelViewProjMatrix * position;\n"
"    out.texCoord = in.texCoord;\n"
"    return out;\n"
"}\n"
"\n"
"fragment float4 fragmentShader(ColorInOut in [[stage_in]],\n"
"                               constant Uniforms & uniforms [[ buffer(2) ]],\n"
"                               texture2d<half> colorMap [[ texture(0) ]]) {\n"
"    constexpr sampler colorSampler(mip_filter::linear, mag_filter::linear, min_filter::linear);\n"
"    half4 colorSample = colorMap.sample(colorSampler, in.texCoord.xy);\n"
"    return float4(colorSample);\n"
"}\n";
