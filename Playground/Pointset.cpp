#include "pch.h"

#include "Pointset.h"
#include "rendering.h"

namespace Playground {

using namespace Gfx;

Pointset::PointHandle Pointset::Add(Vector3 position, float radius, Color4 colour)
{
    i32 index = freelist_.Allocate();
    indirection_.Reserve(index + 1);
    rev_indirection_.Reserve(index + 1);

    if(indirection_.Size() == index) {
        indirection_.PushBack(-1);
        rev_indirection_.PushBack(-1);
    }

    plgr_assert(indirection_[index] == -1);
    indirection_[index] = As<i32>(points_.Size());
    plgr_assert(rev_indirection_[points_.Size()] == -1);
    rev_indirection_[points_.Size()] = index;

    points_.PushBack({ .position = position, .size = radius, .prev_position = position, .colour = colour });

    dirty_ = true;

    return { index };
}

void Pointset::Remove(PointHandle h) 
{
    i32 i = indirection_[h.index];
    i32 last = As<i32>(points_.Size() - 1);
    plgr_assert(last >= 0);
    points_.RemoveAtAndSwapWithLast(i);
    plgr_assert(rev_indirection_[last] >= 0);
    plgr_assert(indirection_[rev_indirection_[last]] >= 0);
    i32 handle_last = rev_indirection_[last];
    indirection_[handle_last] = i;
    indirection_[h.index] = -1;
    rev_indirection_[i] = handle_last;
    rev_indirection_[last] = -1;
    freelist_.Free(h.index);
}

void Pointset::Modify(PointHandle h, Optional<Vector3> new_position, Optional<float> new_size) 
{
    PointPayload & p = points_[indirection_[h.index]];
    if(new_position) {
        p.prev_position = p.position;
        p.position = *new_position;
    }
    if(new_size) {
        p.size = *new_size;
    }

    dirty_ = true;
}

i64 Pointset::Size() const
{
    return points_.Size();
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

    if (pointset_->dirty_&& pointset_->Size()) {
        update_pass_ = device_->graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                             .Attach({ .resource = *points_buffer_->resource_ }, D3D12_RESOURCE_STATE_COPY_DEST));
    }

    particle_pass_ = device_->graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                           .Attach({ .resource = *points_buffer_->resource_ }, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
                                                           .Attach({ .resource = *depth_buffer_->resource_ }, D3D12_RESOURCE_STATE_DEPTH_WRITE)
                                                           .Attach({ .resource = *colour_target_->resource_ }, D3D12_RESOURCE_STATE_RENDER_TARGET)
                                                           .Attach({ .resource = *motionvec_target_->resource_ }, D3D12_RESOURCE_STATE_RENDER_TARGET));
}

void PointsetRenderer::Render(Gfx::Encoder* encoder, ViewportRenderContext const * viewport_ctx, D3D12_CPU_DESCRIPTOR_HANDLE* rtv_handles, D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle)
{
    if (!points_buffer_) {
        return;
    }

    const i64 points_num = pointset_->points_.Size();

    if (pointset_->dirty_ && pointset_->Size()) {
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