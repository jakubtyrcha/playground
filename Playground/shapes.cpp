#include "pch.h"

#include "rendering.h"
#include "shader.h"
#include "shapes.h"

namespace Rendering {
struct ShapesPipeline : public Gfx::IPipelineBuilder {
    ImmediateModeShapeRenderer* owner_ = nullptr;

    ShapesPipeline(ImmediateModeShapeRenderer* owner)
        : owner_(owner)
    {
    }

    Box<Gfx::Pipeline> Build() override
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc {};
        pso_desc.NodeMask = 1;
        pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        pso_desc.pRootSignature = *owner_->device_->root_signature_;
        pso_desc.SampleMask = UINT_MAX;
        pso_desc.NumRenderTargets = 1;
        pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        pso_desc.SampleDesc.Count = 1;
        pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

        pso_desc.VS = GetShaderFromShaderFileSource({ .file_path = L"../data/shape.hlsl",
                                                        .entrypoint = L"VsMain",
                                                        .profile = L"vs_6_0" })
                          ->GetBytecode();

        pso_desc.PS = GetShaderFromShaderFileSource({ .file_path = L"../data/shape.hlsl",
                                                        .entrypoint = L"PsMain",
                                                        .profile = L"ps_6_0" })
                          ->GetBytecode();

        // Create the input layout
        static D3D12_INPUT_ELEMENT_DESC local_layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };
        pso_desc.InputLayout = { local_layout, _countof(local_layout) };

        // Create the blending setup
        {
            D3D12_BLEND_DESC& desc = pso_desc.BlendState;
            desc.AlphaToCoverageEnable = false;
            desc.RenderTarget[0].BlendEnable = false;
            desc.RenderTarget[0].LogicOpEnable = false;
            desc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        }

        // Create the rasterizer state
        {
            D3D12_RASTERIZER_DESC& desc = pso_desc.RasterizerState;
            desc.FillMode = D3D12_FILL_MODE_SOLID;
            desc.CullMode = D3D12_CULL_MODE_NONE;
            desc.FrontCounterClockwise = FALSE;
            desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
            desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            desc.DepthClipEnable = true;
            desc.MultisampleEnable = FALSE;
            desc.AntialiasedLineEnable = FALSE;
            desc.ForcedSampleCount = 0;
            desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        }

        // Create depth-stencil State
        {
            D3D12_DEPTH_STENCIL_DESC& desc = pso_desc.DepthStencilState;
            desc.DepthEnable = false; //
            desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            desc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
            desc.StencilEnable = false;
            desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
            desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            desc.BackFace = desc.FrontFace;
        }

        return Gfx::Pipeline::From(owner_->device_, pso_desc);
    }
};

void ImmediateModeShapeRenderer::Init(Gfx::Device* device)
{
    device_ = device;

    pipeline_ = MakeBox<ShapesPipeline>(this);
}

void ImmediateModeShapeRenderer::Render(Gfx::Encoder* encoder, Viewport* viewport, D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle)
{
    while (frame_data_queue_.Size() && frame_data_queue_.First().waitable_.IsDone()) {
        frame_data_queue_.RemoveAt(0);
    }

    encoder->GetCmdList()->OMSetRenderTargets(1, &rtv_handle, false, nullptr);

    D3D12_VIEWPORT vp {
        .Width = static_cast<float>(viewport->resolution.x()),
        .Height = static_cast<float>(viewport->resolution.y()),
        .MinDepth = 0.f,
        .MaxDepth = 1.f
    };
    encoder->GetCmdList()->RSSetViewports(1, &vp);

    static_assert(offsetof(ShapeVertex, colour) == 12);

    i64 verticesNum = vertices_.Size();

    FrameData frame_data;
    frame_data.vertex_buffer_ = device_->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, verticesNum * sizeof(ShapeVertex), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ);

    ShapeVertex* vtx_dst = nullptr;
    verify_hr(frame_data.vertex_buffer_.resource_->Map(0, nullptr, reinterpret_cast<void**>(&vtx_dst)));
    memcpy(vtx_dst, vertices_.Data(), verticesNum * sizeof(ShapeVertex));
    frame_data.vertex_buffer_.resource_->Unmap(0, nullptr);

    // Bind shader and vertex buffers
    unsigned int stride = sizeof(ShapeVertex);
    unsigned int offset = 0;
    D3D12_VERTEX_BUFFER_VIEW vbv {
        .BufferLocation = frame_data.vertex_buffer_.resource_->GetGPUVirtualAddress() + offset,
        .SizeInBytes = static_cast<u32>(verticesNum * stride),
        .StrideInBytes = stride
    };
    encoder->GetCmdList()->IASetVertexBuffers(0, 1, &vbv);
    encoder->GetCmdList()->IASetIndexBuffer(nullptr);
    encoder->GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    encoder->GetCmdList()->SetPipelineState(pipeline_->GetPSO());

    frame_data.constant_buffers_.PushBackRvalueRef(std::move(device_->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, AlignedForward(64, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ)));

    Matrix4x4 view_projection_matrix = viewport->view_projection_matrix;

    void* cb_dst = nullptr;
    verify_hr(frame_data.constant_buffers_.Last().resource_->Map(0, nullptr, &cb_dst));
    memcpy(cb_dst, &view_projection_matrix, 64);
    frame_data.constant_buffers_.Last().resource_->Unmap(0, nullptr);

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc {
        .BufferLocation = frame_data.constant_buffers_.Last().resource_->GetGPUVirtualAddress(),
        .SizeInBytes = static_cast<u32>(AlignedForward(64, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT))
    };
    device_->device_->CreateConstantBufferView(&cbv_desc, encoder->ReserveGraphicsSlot(Gfx::DescriptorType::CBV, 0));

    encoder->SetGraphicsDescriptors();

    const D3D12_RECT r = { 0, 0, viewport->resolution.x(), viewport->resolution.y() };
    encoder->GetCmdList()->RSSetScissorRects(1, &r);
    encoder->GetCmdList()->DrawInstanced(static_cast<u32>(verticesNum), 1, 0, 0);

    frame_data.waitable_ = encoder->GetWaitable();

    frame_data_queue_.PushBackRvalueRef(std::move(frame_data));

    vertices_.Clear();
}

void ImmediateModeShapeRenderer::Shutdown()
{
    frame_data_queue_.Clear();
}

void ImmediateModeShapeRenderer::AddLine(Vector3 a, Vector3 b, ColourR8G8B8A8U colour)
{
    vertices_.PushBack({ .position = a, .colour = colour });
    vertices_.PushBack({ .position = b, .colour = colour });
}
}