#include "pch.h"

#include "Pointset.h"
#include "rendering.h"

using namespace Core;
using namespace Containers;

namespace Rendering {

void Pointset::Add(Vector3 position, float size, Color4ub colour)
{
    points_.PushBack({ .position = position, .size = size, .colour = colour });
}

template <typename RenderComponent>
struct RenderComponentPipeline : public Gfx::IPipelineBuilder {
    RenderComponent* owner_ = nullptr;

    RenderComponentPipeline(RenderComponent* render_component)
        : owner_(render_component)
    {
    }
};

D3D12_GRAPHICS_PIPELINE_STATE_DESC GetDefaultPipelineStateDesc(Gfx::Device* device)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc {};
    pso_desc.NodeMask = 1;
    pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pso_desc.pRootSignature = *device->root_signature_;
    pso_desc.SampleMask = UINT_MAX;
    //pso_desc.NumRenderTargets = 2;
    //pso_desc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    //pso_desc.RTVFormats[1] = DXGI_FORMAT_R16G16_FLOAT;
    //pso_desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    pso_desc.SampleDesc.Count = 1;
    pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    //pso_desc.VS = GetShaderFromShaderFileSource({ .file_path = L"../data/sphere_splatting.hlsl",
    //                                                .entrypoint = L"VsMain",
    //                                                .profile = L"vs_6_0" })
    //                  ->GetBytecode();
    //
    //pso_desc.PS = GetShaderFromShaderFileSource({ .file_path = L"../data/sphere_splatting.hlsl",
    //                                                .entrypoint = L"PsMain",
    //                                                .profile = L"ps_6_0" })
    //                  ->GetBytecode();

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
        desc.DepthEnable = false;
        desc.StencilEnable = false;
    }

    return pso_desc;
}

struct PointsetRendererPipeline : public RenderComponentPipeline<PointsetRenderer> {
    PointsetRendererPipeline(PointsetRenderer* owner)
        : RenderComponentPipeline<PointsetRenderer>(owner)
    {
    }

    Optional<Box<Gfx::Pipeline>> Build() override
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = GetDefaultPipelineStateDesc(owner_->device_);
        pso_desc.NumRenderTargets = 2;
        pso_desc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
        pso_desc.RTVFormats[1] = DXGI_FORMAT_R16G16_FLOAT;
        pso_desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        pso_desc.VS = GetShaderFromShaderFileSource({ .file_path = L"../data/sphere_splatting.hlsl",
                                                        .entrypoint = L"VsMain",
                                                        .profile = L"vs_6_0" })
                          ->GetBytecode();

        pso_desc.PS = GetShaderFromShaderFileSource({ .file_path = L"../data/sphere_splatting.hlsl",
                                                        .entrypoint = L"PsMain",
                                                        .profile = L"ps_6_0" })
                          ->GetBytecode();

        return Gfx::Pipeline::From(owner_->device_, pso_desc);
    }
};

void PointsetRenderer::Init(Gfx::Device* device)
{
    RenderComponent::Init(device);

    pipeline_ = MakeBox<PointsetRendererPipeline>(this);

    points_buffer_ = device->CreateBuffer(D3D12_HEAP_TYPE_DEFAULT, 1, DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);
}

void PointsetRenderer::AddPassesToGraph()
{
    update_pass_ = device_->graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                         .Attach({ .resource = *points_buffer_.resource_ }, D3D12_RESOURCE_STATE_COPY_DEST));

    particle_pass_ = device_->graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                           .Attach({ .resource = *points_buffer_.resource_ }, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
                                                           .Attach({ .resource = *depth_buffer_->resource_ }, D3D12_RESOURCE_STATE_DEPTH_WRITE)
                                                           .Attach({ .resource = *colour_target_->resource_ }, D3D12_RESOURCE_STATE_RENDER_TARGET)
                                                           .Attach({ .resource = *motionvec_target_->resource_ }, D3D12_RESOURCE_STATE_RENDER_TARGET));
}

void PointsetRenderer::Render(Gfx::Encoder* encoder, ViewportRenderContext* viewport_ctx, D3D12_CPU_DESCRIPTOR_HANDLE* rtv_handles, D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle)
{
    while (frame_data_queue_.Size() && frame_data_queue_.First().waitable_.IsDone()) {
        frame_data_queue_.RemoveAt(0);
    }

    FrameData frame_data;

    const i64 points_num = pointset_->points_.Size();

    encoder->SetPass(update_pass_);

    encoder->SetPass(particle_pass_);

    encoder->GetCmdList()->OMSetRenderTargets(2, rtv_handles, false, &dsv_handle);

    D3D12_VIEWPORT vp {
        .Width = static_cast<float>(viewport_ctx->viewport->resolution.x()),
        .Height = static_cast<float>(viewport_ctx->viewport->resolution.y()),
        .MinDepth = 0.f,
        .MaxDepth = 1.f
    };
    encoder->GetCmdList()->RSSetViewports(1, &vp);

    encoder->GetCmdList()->IASetVertexBuffers(0, 1, nullptr);
    encoder->GetCmdList()->IASetIndexBuffer(nullptr);
    encoder->GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    encoder->GetCmdList()->SetPipelineState(pipeline_->GetPSO());
    encoder->SetGraphicsDescriptors();
    const D3D12_RECT r = { 0, 0, viewport_ctx->viewport->resolution.x(), viewport_ctx->viewport->resolution.y() };
    encoder->GetCmdList()->RSSetScissorRects(1, &r);
    encoder->GetCmdList()->DrawInstanced(4, u32(points_num), 0, 0);

    frame_data.waitable_ = encoder->GetWaitable();

    frame_data_queue_.PushBackRvalueRef(std::move(frame_data));
}

}