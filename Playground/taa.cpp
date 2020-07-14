#include "Pch.h"

#include "Rendering.h"
#include "Shader.h"
#include "Taa.h"

using namespace Core;
using namespace Containers;

namespace Rendering {

struct TAAPipeline : public Gfx::IPipelineBuilder {
    TAA* owner_ = nullptr;

    TAAPipeline(TAA* owner)
        : owner_(owner)
    {
    }

    Optional<Box<Gfx::Pipeline>> Build() override
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc {};
        pso_desc.NodeMask = 1;
        pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pso_desc.pRootSignature = *owner_->device_->root_signature_;
        pso_desc.SampleMask = UINT_MAX;
        pso_desc.NumRenderTargets = 1;
        pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        pso_desc.SampleDesc.Count = 1;
        pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

        pso_desc.VS = GetShaderFromShaderFileSource({ .file_path = L"../data/taa.hlsl",
                                                        .entrypoint = L"VSMain",
                                                        .profile = L"vs_6_0" })
                          ->GetBytecode();

        pso_desc.PS = GetShaderFromShaderFileSource({ .file_path = L"../data/taa.hlsl",
                                                        .entrypoint = L"PSMain",
                                                        .profile = L"ps_6_0" })
                          ->GetBytecode();

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

        return Gfx::Pipeline::From(owner_->device_, pso_desc);
    }
};

void TAA::Init(Gfx::Device* device)
{
    device_ = device;

    pipeline_ = MakeBox<TAAPipeline>(this);
}

void TAA::AddPassesToGraph(ID3D12Resource* colour_target, Gfx::Resource* colour_src, Gfx::Resource* depth_src, Gfx::Resource* prev_colour_src)
{
    taa_pass_ = device_->graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                      .Attach({ .resource = colour_target }, D3D12_RESOURCE_STATE_RENDER_TARGET)
                                                      .Attach({ .resource = *colour_src->resource_ }, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE|D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
                                                      .Attach({ .resource = *depth_src->resource_ }, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE|D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
                                                      .Attach({ .resource = *prev_colour_src->resource_ }, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE|D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    colour_src_ = colour_src;
    depth_src_ = depth_src;
    prev_colour_src_ = prev_colour_src;
}

void TAA::Render(Gfx::Encoder* encoder, Viewport* viewport, D3D12_CPU_DESCRIPTOR_HANDLE colour_target_handle)
{
    while (frame_data_queue_.Size() && frame_data_queue_.First().waitable_.IsDone()) {
        frame_data_queue_.RemoveAt(0);
    }

    FrameData frame_data;

    struct FrameConstants {
        Vector2i resolution;
        Vector2 inv_resolution;
        Vector2 near_far_planes;
        Vector2 clipspace_jitter;
        Matrix4 view_matrix;
        Matrix4 projection_matrix;
        Matrix4 view_projection_matrix;
        Matrix4 inv_view_projection_matrix;
        Matrix4 inv_view_matrix;
        Matrix4 prev_view_projection_matrix;
        float taa_history_decay;
    };
    FrameConstants constants;
    constants.resolution = viewport->resolution;
    constants.inv_resolution = 1.f / Vector2 { constants.resolution };
    constants.near_far_planes = Vector2 { viewport->near_plane, viewport->far_plane };
    if (viewport->taa_offsets.Size()) {
        constants.clipspace_jitter = viewport->projection_jitter;
    } else {
        constants.clipspace_jitter = {};
    }
    constants.view_matrix = viewport->view_matrix;
    constants.projection_matrix = viewport->projection_matrix;
    constants.view_projection_matrix = viewport->view_projection_matrix;
    constants.inv_view_projection_matrix = viewport->inv_view_projection_matrix;
    constants.inv_view_matrix = viewport->inv_view_matrix;
    constants.prev_view_projection_matrix = viewport->prev_view_projection_matrix;
    constants.taa_history_decay = viewport->history_decay;

    frame_data.constant_buffers_.PushBackRvalueRef(std::move(device_->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, AlignedForward(static_cast<i32>(sizeof(FrameConstants)), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ)));
    void* cb_dst = nullptr;
    verify_hr(frame_data.constant_buffers_.Last().resource_->Map(0, nullptr, &cb_dst));
    memcpy(cb_dst, &constants, sizeof(FrameConstants));
    frame_data.constant_buffers_.Last().resource_->Unmap(0, nullptr);

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc {
        .BufferLocation = frame_data.constant_buffers_.Last().resource_->GetGPUVirtualAddress(),
        .SizeInBytes = static_cast<u32>(AlignedForward(static_cast<i32>(sizeof(FrameConstants)), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT))
    };
    device_->device_->CreateConstantBufferView(&cbv_desc, encoder->ReserveGraphicsSlot(Gfx::DescriptorType::CBV, 0));

    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc {
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Texture2D = {
                .MipLevels = 1 }
        };
        device_->device_->CreateShaderResourceView(*colour_src_->resource_, &srv_desc, encoder->ReserveGraphicsSlot(Gfx::DescriptorType::SRV, 0));
    }
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc {
            .Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
            .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Texture2D = {
                .MipLevels = 1 }
        };
        device_->device_->CreateShaderResourceView(*depth_src_->resource_, &srv_desc, encoder->ReserveGraphicsSlot(Gfx::DescriptorType::SRV, 1));
    }
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc {
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Texture2D = {
                .MipLevels = 1 }
        };
        device_->device_->CreateShaderResourceView(*prev_colour_src_->resource_, &srv_desc, encoder->ReserveGraphicsSlot(Gfx::DescriptorType::SRV, 2));
    }

    encoder->SetPass(taa_pass_);
    taa_pass_ = nullptr;

    encoder->GetCmdList()->OMSetRenderTargets(1, &colour_target_handle, false, nullptr);

    D3D12_VIEWPORT vp {
        .Width = static_cast<float>(viewport->resolution.x()),
        .Height = static_cast<float>(viewport->resolution.y()),
        .MinDepth = 0.f,
        .MaxDepth = 1.f
    };
    encoder->GetCmdList()->RSSetViewports(1, &vp);

    encoder->GetCmdList()->IASetVertexBuffers(0, 1, nullptr);
    encoder->GetCmdList()->IASetIndexBuffer(nullptr);
    encoder->GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    encoder->GetCmdList()->SetPipelineState(pipeline_->GetPSO());
    encoder->SetGraphicsDescriptors();
    const D3D12_RECT r = { 0, 0, viewport->resolution.x(), viewport->resolution.y() };
    encoder->GetCmdList()->RSSetScissorRects(1, &r);
    encoder->GetCmdList()->DrawInstanced(3, 1, 0, 0);

    frame_data.waitable_ = encoder->GetWaitable();

    frame_data_queue_.PushBackRvalueRef(std::move(frame_data));
}

}