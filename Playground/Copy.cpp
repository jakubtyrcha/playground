#include "Pch.h"

#include "Rendering.h"
#include "Shader.h"
#include "Copy.h"

using namespace Core;
using namespace Containers;

namespace Rendering {

struct CopyPipeline : public Gfx::IPipelineBuilder {
    Copy* owner_ = nullptr;

    CopyPipeline(Copy* owner)
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

        pso_desc.VS = GetShaderFromShaderFileSource({ .file_path = L"../data/copy.hlsl",
                                                        .entrypoint = L"VSMain",
                                                        .profile = L"vs_6_0" })
                          ->GetBytecode();

        pso_desc.PS = GetShaderFromShaderFileSource({ .file_path = L"../data/copy.hlsl",
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

void Copy::Init(Gfx::Device* device)
{
    device_ = device;

    pipeline_ = MakeBox<CopyPipeline>(this);
}

void Copy::AddPassesToGraph(ID3D12Resource* colour_target, Gfx::Resource* colour_src)
{
    pass_ = device_->graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                      .Attach({ .resource = colour_target }, D3D12_RESOURCE_STATE_RENDER_TARGET)
                                                      .Attach({ .resource = *colour_src->resource_ }, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE|D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
    );

    colour_src_ = colour_src;
}

void Copy::Render(Gfx::Encoder* encoder, Viewport* viewport, D3D12_CPU_DESCRIPTOR_HANDLE colour_target_handle)
{
    while (frame_data_queue_.Size() && frame_data_queue_.First().waitable_.IsDone()) {
        frame_data_queue_.RemoveAt(0);
    }

    FrameData frame_data;

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

    encoder->SetPass(pass_);
    pass_ = nullptr;

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