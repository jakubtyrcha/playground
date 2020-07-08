#include "pch.h"

#include "imgui_backend.h"
#include "shader.h"
#include <imgui/imgui.h>

namespace Rendering {

struct ImGuiPipeline : public Gfx::IPipelineBuilder {
    ImGuiRenderer* owner_ = nullptr;

    ImGuiPipeline(ImGuiRenderer* owner)
        : owner_(owner)
    {
    }

    Box<Gfx::Pipeline> Build() override
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

        return Gfx::Pipeline::From(owner_->device_, pso_desc);
    }
};

void ImGuiRenderer::Init(Gfx::Device* device)
{
    device_ = device;

    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = "imgui_impl_playground";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    //io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;

    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    main_viewport->RendererUserData = &main_viewport_;

    // Setup back-end capabilities flags
    //io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
    //if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    //	ImGui_ImplDX12_InitPlatformInterface();

    _CreateFontsTexture();

    pipeline_ = MakeBox<ImGuiPipeline>(this);
}

void ImGuiRenderer::Shutdown()
{
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    main_viewport->RendererUserData = nullptr;

    frame_data_queue_.Clear();
}

void ImGuiRenderer::_CreateFontsTexture()
{
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    font_texture_ = device_->CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, { width, height }, DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);

    i32 upload_pitch = AlignedForward(width * 4, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
    D3D12_RESOURCE_DESC desc = font_texture_.resource_->GetDesc();
    Gfx::Resource upload_buffer = device_->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, width * upload_pitch, DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ);

    u8* mapped_memory = nullptr;
    verify_hr(upload_buffer.resource_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_memory)));

    for (i64 r = 0; r < height; r++) {
        memcpy(mapped_memory + r * upload_pitch, pixels + r * width * 4, width * 4);
    }

    upload_buffer.resource_->Unmap(0, nullptr);

    Gfx::Encoder encoder = device_->CreateEncoder();
    D3D12_TEXTURE_COPY_LOCATION copy_src {
        .pResource = *upload_buffer.resource_,
        .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
        .PlacedFootprint = {
            .Footprint = {
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .Width = static_cast<u32>(width),
                .Height = static_cast<u32>(height),
                .Depth = 1,
                .RowPitch = static_cast<u32>(upload_pitch) } }
    };
    D3D12_TEXTURE_COPY_LOCATION copy_dst {
        .pResource = *font_texture_.resource_,
        .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
        .SubresourceIndex = 0
    };
    encoder.GetCmdList()->CopyTextureRegion(&copy_dst, 0, 0, 0, &copy_src, nullptr);

    Gfx::Pass* transition_to_readable_pass = device_->graph_.AddSubsequentPass(Gfx::PassAttachments {}.Attach({ .resource = *font_texture_.resource_ }, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
    encoder.SetPass(transition_to_readable_pass);

    encoder.Submit();
    device_->GetWaitable().Wait();

    // Store our identifier
    //static_assert(sizeof(ImTextureID) >= sizeof(g_hFontSrvGpuDescHandle.ptr), "Can't pack descriptor handle into TexID, 32-bit not supported yet.");
    //io.Fonts->TexID = (ImTextureID)g_hFontSrvGpuDescHandle.ptr;
}

void ImGuiRenderer::_SetupRenderState(ImDrawData* draw_data, Gfx::Encoder* encoder, FrameData* frame_data)
{
    float L = draw_data->DisplayPos.x;
    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    float T = draw_data->DisplayPos.y;
    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
    float mvp[4][4] = {
        { 2.0f / (R - L), 0.0f, 0.0f, 0.0f },
        { 0.0f, 2.0f / (T - B), 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.5f, 0.0f },
        { (R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f },
    };

    // Setup viewport
    D3D12_VIEWPORT vp;
    memset(&vp, 0, sizeof(D3D12_VIEWPORT));
    vp.Width = draw_data->DisplaySize.x;
    vp.Height = draw_data->DisplaySize.y;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = vp.TopLeftY = 0.0f;
    encoder->GetCmdList()->RSSetViewports(1, &vp);

    // Bind shader and vertex buffers
    unsigned int stride = sizeof(ImDrawVert);
    unsigned int offset = 0;
    D3D12_VERTEX_BUFFER_VIEW vbv;
    memset(&vbv, 0, sizeof(D3D12_VERTEX_BUFFER_VIEW));
    vbv.BufferLocation = frame_data->vertex_buffer_.resource_->GetGPUVirtualAddress() + offset;
    vbv.SizeInBytes = draw_data->TotalVtxCount * stride;
    vbv.StrideInBytes = stride;
    encoder->GetCmdList()->IASetVertexBuffers(0, 1, &vbv);
    D3D12_INDEX_BUFFER_VIEW ibv;
    memset(&ibv, 0, sizeof(D3D12_INDEX_BUFFER_VIEW));
    ibv.BufferLocation = frame_data->index_buffer_.resource_->GetGPUVirtualAddress();
    ibv.SizeInBytes = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
    ibv.Format = sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
    encoder->GetCmdList()->IASetIndexBuffer(&ibv);
    encoder->GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    encoder->GetCmdList()->SetPipelineState(pipeline_->GetPSO());

    frame_data->constant_buffers_.PushBackRvalueRef(std::move(device_->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, AlignedForward(64, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ)));

    void* cb_dst = nullptr;
    verify_hr(frame_data->constant_buffers_.Last().resource_->Map(0, nullptr, &cb_dst));
    memcpy(cb_dst, &mvp[0][0], 64);
    frame_data->constant_buffers_.Last().resource_->Unmap(0, nullptr);

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc {
        .BufferLocation = frame_data->constant_buffers_.Last().resource_->GetGPUVirtualAddress(),
        .SizeInBytes = static_cast<u32>(AlignedForward(64, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT))
    };
    device_->device_->CreateConstantBufferView(&cbv_desc, encoder->ReserveGraphicsSlot(Gfx::DescriptorType::CBV, 0));

    // Setup blend factor
    const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
    encoder->GetCmdList()->OMSetBlendFactor(blend_factor);
}

void ImGuiRenderer::RenderDrawData(ImDrawData* draw_data, Gfx::Encoder* encoder, D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle)
{
    if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
        return;

    if (draw_data->TotalVtxCount == 0 || draw_data->TotalIdxCount == 0) {
        return;
    }

    while (frame_data_queue_.Size() && frame_data_queue_.First().waitable_.IsDone()) {
        frame_data_queue_.RemoveAt(0);
    }

    ViewportData* render_data = (ViewportData*)draw_data->OwnerViewport->RendererUserData;

    encoder->GetCmdList()->OMSetRenderTargets(1, &rtv_handle, false, nullptr);

    FrameData frame_data;
    frame_data.vertex_buffer_ = device_->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, draw_data->TotalVtxCount * sizeof(ImDrawVert), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ);
    frame_data.index_buffer_ = device_->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, draw_data->TotalIdxCount * sizeof(ImDrawIdx), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ);

    ImDrawVert* vtx_dst = nullptr;
    ImDrawIdx* idx_dst = nullptr;
    verify_hr(frame_data.vertex_buffer_.resource_->Map(0, nullptr, reinterpret_cast<void**>(&vtx_dst)));
    verify_hr(frame_data.index_buffer_.resource_->Map(0, nullptr, reinterpret_cast<void**>(&idx_dst)));

    for (i32 n = 0, N = draw_data->CmdListsCount; n < N; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];

        memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));

        vtx_dst += cmd_list->VtxBuffer.Size;
        idx_dst += cmd_list->IdxBuffer.Size;
    }

    frame_data.vertex_buffer_.resource_->Unmap(0, nullptr);
    frame_data.index_buffer_.resource_->Unmap(0, nullptr);

    _SetupRenderState(draw_data, encoder, &frame_data);

    int global_vtx_offset = 0;
    int global_idx_offset = 0;
    ImVec2 clip_off = draw_data->DisplayPos;
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != nullptr) {
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState) {
                    _SetupRenderState(draw_data, encoder, &frame_data);
                } else {
                    pcmd->UserCallback(cmd_list, pcmd);
                }
            } else {
                // Apply Scissor, Bind texture, Draw
                const D3D12_RECT r = { (LONG)(pcmd->ClipRect.x - clip_off.x), (LONG)(pcmd->ClipRect.y - clip_off.y), (LONG)(pcmd->ClipRect.z - clip_off.x), (LONG)(pcmd->ClipRect.w - clip_off.y) };
                //ctx->SetGraphicsRootDescriptorTable(1, *(D3D12_GPU_DESCRIPTOR_HANDLE*)&pcmd->TextureId);
                D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc {
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                    .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
                    .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
                    .Texture2D = { .MipLevels = 1 }
                };
                device_->device_->CreateShaderResourceView(*font_texture_.resource_, &srv_desc, encoder->ReserveGraphicsSlot(Gfx::DescriptorType::SRV, 0));
                encoder->GetCmdList()->RSSetScissorRects(1, &r);
                encoder->SetGraphicsDescriptors();
                encoder->GetCmdList()->DrawIndexedInstanced(pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
            }
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }

    frame_data.waitable_ = encoder->GetWaitable();

    frame_data_queue_.PushBackRvalueRef(std::move(frame_data));
}

}