#include "pch.h"

#include "Heightfield.h"
#include "rendering.h"

namespace Playground {

using namespace Gfx;

namespace Gfx {
    void UpdateBuffer(Device* device, Resource* resource, const void* src, i32 bytes, Optional<D3D12_RESOURCE_STATES> post_transition)
    {
        i32 upload_pitch = AlignedForward(bytes, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
        i64 upload_size = bytes;

        Gfx::Resource upload_buffer = device->CreateBuffer(
            D3D12_HEAP_TYPE_UPLOAD,
            upload_size,
            DXGI_FORMAT_UNKNOWN,
            D3D12_RESOURCE_FLAG_NONE,
            D3D12_RESOURCE_STATE_GENERIC_READ);

        u8* mapped_memory = nullptr;
        verify_hr(upload_buffer.resource_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_memory)));
        memcpy(mapped_memory, src, bytes);
        upload_buffer.resource_->Unmap(0, nullptr);

        Gfx::Encoder encoder = device->CreateEncoder();

        Gfx::Pass* copy_to_pass = device->graph_.AddSubsequentPass(Gfx::PassAttachments {}.Attach({ .resource = *resource->resource_ }, D3D12_RESOURCE_STATE_COPY_DEST));
        encoder.SetPass(copy_to_pass);
        encoder.GetCmdList()->CopyBufferRegion(*resource->resource_, 0, *upload_buffer.resource_, 0, bytes);

        if (post_transition) {
            Gfx::Pass* transition_to_readable_pass = device->graph_.AddSubsequentPass(Gfx::PassAttachments {}.Attach({ .resource = *resource->resource_ }, *post_transition));
            encoder.SetPass(transition_to_readable_pass);
        }
        encoder.Submit();

        device->ReleaseWhenCurrentFrameIsDone(std::move(upload_buffer));
    }
}

struct HeightfieldRendererPipeline : public RenderComponentPipeline<HeightfieldRenderer> {
    HeightfieldRendererPipeline(HeightfieldRenderer* owner)
        : RenderComponentPipeline<HeightfieldRenderer>(owner)
    {
    }

    Optional<Box<Gfx::Pipeline>> Build() override
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = GetDefaultPipelineStateDesc(owner_->device_);
        pso_desc.NumRenderTargets = 2;
        pso_desc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
        pso_desc.RTVFormats[1] = DXGI_FORMAT_R16G16_FLOAT;
        pso_desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        pso_desc.VS = GetShaderFromShaderFileSource({ .file_path = L"../data/heightfield.hlsl",
                                                        .entrypoint = L"VsMain",
                                                        .profile = L"vs_6_0" })
                          ->GetBytecode();

        pso_desc.PS = GetShaderFromShaderFileSource({ .file_path = L"../data/heightfield.hlsl",
                                                        .entrypoint = L"PsMain",
                                                        .profile = L"ps_6_0" })
                          ->GetBytecode();

        pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;

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



void HeightfieldRenderer::CreateIB()
{
    plgr_assert(resolution_.x() > 0);

    Array<u32> indices;

    // triangle list
    for (i32 y = 0; y < resolution_.y() - 1; y++) {
        for (i32 x = 0; x < resolution_.x() - 1; x++) {
            i32 v0 = resolution_.x() * y + x;
            i32 v1 = resolution_.x() * (y + 1) + x;
            i32 v2 = resolution_.x() * (y + 1) + x + 1;
            i32 v3 = resolution_.x() * y + x + 1;            

            indices.PushBack(v0);
            indices.PushBack(v1);
            indices.PushBack(v2);
            indices.PushBack(v0);
            indices.PushBack(v2);
            indices.PushBack(v3);
        }
    }


    index_buffer_ = device_->CreateBuffer(D3D12_HEAP_TYPE_DEFAULT, indices.Size() * sizeof(u32), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);

    UpdateBuffer(device_, &index_buffer_, indices.Data(), As<i32>(indices.Size() * sizeof(u32)), D3D12_RESOURCE_STATE_INDEX_BUFFER);
}

void HeightfieldRenderer::Init(Gfx::Device* device)
{
    RenderComponent::Init(device);

    pipeline_ = MakeBox<HeightfieldRendererPipeline>(this);
}

void HeightfieldRenderer::AddPassesToGraph()
{
    pass_ = device_->graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                  .Attach({ .resource = *index_buffer_.resource_ }, D3D12_RESOURCE_STATE_INDEX_BUFFER)
                                                  .Attach({ .resource = *heightfield_texture_->resource_ }, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
                                                  .Attach({ .resource = *depth_buffer_->resource_ }, D3D12_RESOURCE_STATE_DEPTH_WRITE)
                                                  .Attach({ .resource = *colour_target_->resource_ }, D3D12_RESOURCE_STATE_RENDER_TARGET)
                                                  .Attach({ .resource = *motionvec_target_->resource_ }, D3D12_RESOURCE_STATE_RENDER_TARGET));
}

struct HeightFieldConstantsGPU {
    Vector3 aabb_min;
    i32 resolution_x;
    Vector3 aabb_max;
    i32 _padding;
    Vector2 inv_resolution_xz;
    i32 _padding1[2];
    Vector3 light_dir;
};

namespace Gfx {
    template <typename T>
    DescriptorHandle MakeFrameCbv(Device* device, Resource* resource, i32 data_count)
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc {
            .BufferLocation = resource->resource_->GetGPUVirtualAddress(),
            .SizeInBytes = static_cast<u32>(AlignedForward(static_cast<i32>(sizeof(T)) * data_count, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT))
        };

        return device->CreateDescriptor(cbv_desc, Lifetime::Frame);
    }

    template <typename T>
    DescriptorHandle MakeFrameCbvFromMemory(Device* device, const T& ptr)
    {
        Resource cbuffer = device->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, AlignedForward(static_cast<i32>(sizeof(T)), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ);

        T* mapped_ptr = nullptr;
        verify_hr(cbuffer.resource_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_ptr)));
        memcpy(mapped_ptr, &ptr, sizeof(T));
        cbuffer.resource_->Unmap(0, nullptr);

        DescriptorHandle result = MakeFrameCbv<T>(device, &cbuffer, 1);

        device->ReleaseWhenCurrentFrameIsDone(std::move(cbuffer));

        return result;
    }

}

void HeightfieldRenderer::Render(Gfx::Encoder* encoder, ViewportRenderContext const * viewport_ctx, D3D12_CPU_DESCRIPTOR_HANDLE* rtv_handles, D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle)
{
    i32 triangles = (resolution_.x() - 1) * (resolution_.y() - 1) * 2;

    encoder->SetPass(pass_);

    encoder->GetCmdList()->OMSetRenderTargets(2, rtv_handles, false, &dsv_handle);

    viewport_ctx->SetViewportAndScissorRect(encoder);
    encoder->GetCmdList()->IASetVertexBuffers(0, 1, nullptr);
    D3D12_INDEX_BUFFER_VIEW ibv {
        .BufferLocation = index_buffer_.resource_->GetGPUVirtualAddress(),
        .SizeInBytes = triangles * 3 * sizeof(u32),
        .Format = DXGI_FORMAT_R32_UINT
    };
    encoder->GetCmdList()->IASetIndexBuffer(&ibv);
    encoder->GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    encoder->GetCmdList()->SetPipelineState(pipeline_->GetPSO());

    HeightFieldConstantsGPU constants {};
    constants.aabb_min = bounding_box_.vec_min;
    constants.aabb_max = bounding_box_.vec_max;
    constants.inv_resolution_xz = Vector2 { 1.f, 1.f } / Vector2 { resolution_ - Vector2i { 1, 1 } };
    constants.resolution_x = resolution_.x();
    constants.light_dir = light_dir_;

    encoder->SetGraphicsDescriptor(DescriptorType::CBV, 0, viewport_ctx->frame_cbv_handle);
    encoder->SetGraphicsDescriptor(DescriptorType::CBV, 1, MakeFrameCbvFromMemory(device_, constants));
    encoder->SetGraphicsDescriptor(DescriptorType::SRV, 0, heightfield_srv_);
    encoder->SetGraphicsDescriptor(DescriptorType::SRV, 1, horizon_angle_srv_);

    encoder->SetGraphicsDescriptors();
    encoder->GetCmdList()->DrawIndexedInstanced(triangles * 3, 1, 0, 0, 0);
}

}