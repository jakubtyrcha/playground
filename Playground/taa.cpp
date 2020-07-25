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
        pso_desc.NumRenderTargets = 2;
        pso_desc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
        pso_desc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
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

void TAA::AddPassesToGraph(Gfx::Resource* colour_copy_target, ID3D12Resource* colour_target, Gfx::Resource* colour_src, Gfx::Resource* depth_src, Gfx::Resource* prev_colour_src, Gfx::Resource* motion_vector_src)
{
    pass_ = device_->graph_.AddSubsequentPass(Gfx::PassAttachments {}
        .Attach({ .resource = *colour_copy_target->resource_ }, D3D12_RESOURCE_STATE_RENDER_TARGET)
                                                      .Attach({ .resource = colour_target }, D3D12_RESOURCE_STATE_RENDER_TARGET)
                                                      .Attach({ .resource = *colour_src->resource_ }, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE|D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
                                                      .Attach({ .resource = *depth_src->resource_ }, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE|D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
                                                      .Attach({ .resource = *prev_colour_src->resource_ }, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE|D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
        .Attach({ .resource = *motion_vector_src->resource_ }, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE|D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
    );

    colour_src_ = colour_src;
    depth_src_ = depth_src;
    prev_colour_src_ = prev_colour_src;
    motion_vector_src_ = motion_vector_src;
}

void TAA::Render(Gfx::Encoder* encoder, ViewportRenderContext* viewport_ctx, D3D12_CPU_DESCRIPTOR_HANDLE colour_copy_target_handle, D3D12_CPU_DESCRIPTOR_HANDLE colour_target_handle)
{
    encoder->SetGraphicsDescriptor(Gfx::DescriptorType::CBV, 0, viewport_ctx->frame_cbv_handle);

    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc {
            .Format = DXGI_FORMAT_R16G16B16A16_FLOAT,
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
            .Format = DXGI_FORMAT_R16G16B16A16_FLOAT,
            .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Texture2D = {
                .MipLevels = 1 }
        };
        device_->device_->CreateShaderResourceView(*prev_colour_src_->resource_, &srv_desc, encoder->ReserveGraphicsSlot(Gfx::DescriptorType::SRV, 2));
    }
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc {
            .Format = DXGI_FORMAT_R16G16_FLOAT,
            .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Texture2D = {
                .MipLevels = 1 }
        };
        device_->device_->CreateShaderResourceView(*motion_vector_src_->resource_, &srv_desc, encoder->ReserveGraphicsSlot(Gfx::DescriptorType::SRV, 3));
    }

    encoder->SetPass(pass_);
    pass_ = nullptr;

    D3D12_CPU_DESCRIPTOR_HANDLE rtv_handles[] = {colour_copy_target_handle, colour_target_handle};
    encoder->GetCmdList()->OMSetRenderTargets(_countof(rtv_handles), rtv_handles, false, nullptr);

    viewport_ctx->SetViewportAndScissorRect(encoder);

    encoder->GetCmdList()->IASetVertexBuffers(0, 1, nullptr);
    encoder->GetCmdList()->IASetIndexBuffer(nullptr);
    encoder->GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    encoder->GetCmdList()->SetPipelineState(pipeline_->GetPSO());
    encoder->SetGraphicsDescriptors();
    encoder->GetCmdList()->DrawInstanced(3, 1, 0, 0);
}

}