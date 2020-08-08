#include "Pch.h"
#include "ImGuiBackend.h"
#include "ShaderSource.h"
#include "Gfx.h"
#define ImTextureID Playground::Gfx::DescriptorHandle
#include <imgui/imgui.h>

namespace Playground {

using namespace Gfx;

struct ImGuiPipeline : public ShaderPipeline<ImGuiShader> {
    ImGuiPipeline(ImGuiShader* owner)
        : ShaderPipeline<ImGuiShader>(owner)
    {
    }

    Optional<Box<Pipeline>> Build() override
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = GetDefaultPipelineStateDesc(owner_->device_);
        pso_desc.NumRenderTargets = 1;
        // TODO: query
        pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

        {
            static const char* vertex_shader = "cbuffer vertexBuffer : register(b0) \
				{\
				  float4x4 ProjectionMatrix; \
				};\
				struct VS_INPUT\
				{\
				  float2 pos : POSITION;\
				  float4 col : COLOR0;\
				  float2 uv  : TEXCOORD0;\
				};\
				\
				struct PS_INPUT\
				{\
				  float4 pos : SV_POSITION;\
				  float4 col : COLOR0;\
				  float2 uv  : TEXCOORD0;\
				};\
				\
				PS_INPUT main(VS_INPUT input)\
				{\
				  PS_INPUT output;\
				  output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\
				  output.col = input.col;\
				  output.uv  = input.uv;\
				  return output;\
				}";

            pso_desc.VS = GetShaderFromStaticSource({ .source = vertex_shader,
                                                        .entrypoint = L"main",
                                                        .profile = L"vs_6_0" })
                              ->GetBytecode();
        }

        // Create the input layout
        static D3D12_INPUT_ELEMENT_DESC local_layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, (UINT)IM_OFFSETOF(ImDrawVert, pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, (UINT)IM_OFFSETOF(ImDrawVert, uv), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (UINT)IM_OFFSETOF(ImDrawVert, col), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };
        pso_desc.InputLayout = { local_layout, 3 };

        {
            static const char* pixel_shader = "struct PS_INPUT\
				{\
				  float4 pos : SV_POSITION;\
				  float4 col : COLOR0;\
				  float2 uv  : TEXCOORD0;\
				};\
				SamplerState sampler0 : register(s0);\
				Texture2D texture0 : register(t0);\
				\
				float4 main(PS_INPUT input) : SV_Target\
				{\
				  float4 out_col = input.col * texture0.Sample(sampler0, input.uv); \
				  return out_col; \
				}";

            pso_desc.PS = GetShaderFromStaticSource({ .source = pixel_shader,
                                                        .entrypoint = L"main",
                                                        .profile = L"ps_6_0" })
                              ->GetBytecode();
        }

        // Create the blending setup
        {
            D3D12_BLEND_DESC& desc = pso_desc.BlendState;
            desc.AlphaToCoverageEnable = false;
            desc.RenderTarget[0].BlendEnable = true;
            desc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            desc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
            desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            desc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
            desc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
            desc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
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
            desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            desc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            desc.StencilEnable = false;
            desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
            desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            desc.BackFace = desc.FrontFace;
        }

        return Pipeline::From(owner_->device_, pso_desc);
    }
};

void ImGuiShader::Init(Gfx::Device* device)
{
    Shader::Init(device);
    pipeline_ = MakeBox<ImGuiPipeline>(this);
}

void ImGuiRenderer::Init(Gfx::Device* device)
{
    owner_ = device;
    shader_ = MakeBox<ImGuiShader>();
    shader_->Init(owner_);

    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = "imgui_impl_playground";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    //io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
    
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    main_viewport->RendererUserData = nullptr;
    
    // Setup back-end capabilities flags
    //io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
    //if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    //	ImGui_ImplDX12_InitPlatformInterface();
    
    _CreateFontsTexture();
}

void ImGuiRenderer::_CreateFontsTexture()
{
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    font_texture_ = owner_->CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, { width, height }, DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);

    UpdateTexture2DSubresource(owner_, &font_texture_, 0, { width, height }, DXGI_FORMAT_R8G8B8A8_UNORM, pixels, width * 4, height);

    font_texture_srv_ = owner_->CreateDescriptor(&font_texture_, D3D12_SHADER_RESOURCE_VIEW_DESC { .Format = DXGI_FORMAT_R8G8B8A8_UNORM, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D, .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING, .Texture2D = { .MipLevels = 1 } },
        Gfx::Lifetime::Manual);

    io.Fonts->TexID = font_texture_srv_;
}

}