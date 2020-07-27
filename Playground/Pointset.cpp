#include "pch.h"

#include "Pointset.h"
#include "rendering.h"

using namespace Core;
using namespace Containers;
using namespace Gfx;

namespace Rendering {

void Pointset::Add(Vector3 position, float size, Color4 colour)
{
    points_.PushBack({ .position = position, .size = size, .prev_position = position, .colour = colour });

    dirty_ = true;
}

i64 Pointset::Size() const
{
    return points_.Size();
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

        {
            D3D12_DEPTH_STENCIL_DESC& desc = pso_desc.DepthStencilState;
            desc.DepthEnable = true;
            desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
            desc.StencilEnable = false;
            desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
            desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            desc.BackFace = desc.FrontFace;
        }

        return Gfx::Pipeline::From(owner_->device_, pso_desc);
    }
};

void PointsetRenderer::Init(Gfx::Device* device)
{
    RenderComponent::Init(device);

    pipeline_ = MakeBox<PointsetRendererPipeline>(this);
}

struct ParticleData_GPU {
    Vector3 position;
    f32 size;
    Vector3 prev_position;
    u32 colour_packed;
};

void PointsetRenderer::AddPassesToGraph()
{
    // TODO: probably not the best spot to resize the buffer, add some tick function?...

    if (i32(current_capacity_) < pointset_->Size()) {
        if (points_buffer_) {
            device_->ReleaseWhenCurrentFrameIsDone(std::move(*points_buffer_));
            device_->ReleaseWhenCurrentFrameIsDone(points_buffer_srv_);
        }

        current_capacity_ = static_cast<u32>(pointset_->Size());

        points_buffer_ = device_->CreateBuffer(D3D12_HEAP_TYPE_DEFAULT, sizeof(ParticleData_GPU) * current_capacity_, DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);

        points_buffer_srv_ = device_->CreateDescriptor(&*points_buffer_, D3D12_SHADER_RESOURCE_VIEW_DESC { .Format = DXGI_FORMAT_UNKNOWN, .ViewDimension = D3D12_SRV_DIMENSION_BUFFER, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .Buffer = {
                                                                                                                                                                                                                                                                .NumElements = static_cast<u32>(current_capacity_),
                                                                                                                                                                                                                                                                .StructureByteStride = sizeof(ParticleData_GPU),
                                                                                                                                                                                                                                                            } },
            Lifetime::Manual);
    }

    if (!points_buffer_) {
        return;
    }

    if (pointset_->dirty_) {
        update_pass_ = device_->graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                             .Attach({ .resource = *points_buffer_->resource_ }, D3D12_RESOURCE_STATE_COPY_DEST));
    }

    particle_pass_ = device_->graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                           .Attach({ .resource = *points_buffer_->resource_ }, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
                                                           .Attach({ .resource = *depth_buffer_->resource_ }, D3D12_RESOURCE_STATE_DEPTH_WRITE)
                                                           .Attach({ .resource = *colour_target_->resource_ }, D3D12_RESOURCE_STATE_RENDER_TARGET)
                                                           .Attach({ .resource = *motionvec_target_->resource_ }, D3D12_RESOURCE_STATE_RENDER_TARGET));
}

void PointsetRenderer::Render(Gfx::Encoder* encoder, ViewportRenderContext* viewport_ctx, D3D12_CPU_DESCRIPTOR_HANDLE* rtv_handles, D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle)
{
    if (!points_buffer_) {
        return;
    }

    const i64 points_num = pointset_->points_.Size();

    if (pointset_->dirty_) {
        encoder->SetPass(update_pass_);

        Gfx::Resource upload_buffer = device_->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, sizeof(ParticleData_GPU) * pointset_->Size(), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ);

        ParticleData_GPU* mapped_ptr = nullptr;
        verify_hr(upload_buffer.resource_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_ptr)));

        for (i64 i = 0, N = pointset_->Size(); i < N; i++) {
            Pointset::PointPayload const& p = pointset_->points_[i];
            mapped_ptr[i] = ParticleData_GPU { .position = p.position, .size = p.size, .prev_position = p.prev_position, .colour_packed = p.colour.toSrgbAlphaInt() };
        }

        upload_buffer.resource_->Unmap(0, nullptr);

        encoder->GetCmdList()->CopyBufferRegion(*points_buffer_->resource_, 0, *upload_buffer.resource_, 0, sizeof(ParticleData_GPU) * pointset_->Size());

        device_->ReleaseWhenCurrentFrameIsDone(std::move(upload_buffer));

        pointset_->dirty_ = false;
    }

    // copy particles?

    encoder->SetPass(particle_pass_);

    encoder->GetCmdList()->OMSetRenderTargets(2, rtv_handles, false, &dsv_handle);

    viewport_ctx->SetViewportAndScissorRect(encoder);

    encoder->GetCmdList()->IASetVertexBuffers(0, 1, nullptr);
    encoder->GetCmdList()->IASetIndexBuffer(nullptr);
    encoder->GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    encoder->GetCmdList()->SetPipelineState(pipeline_->GetPSO());

    encoder->SetGraphicsDescriptor(DescriptorType::CBV, 0, viewport_ctx->frame_cbv_handle);
    encoder->SetGraphicsDescriptor(DescriptorType::SRV, 0, points_buffer_srv_);

    encoder->SetGraphicsDescriptors();
    encoder->GetCmdList()->DrawInstanced(4, u32(points_num), 0, 0);
}

}