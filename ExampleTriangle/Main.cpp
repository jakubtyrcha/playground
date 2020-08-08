#include "Pch.h"
#include "Engine.h"
#include "Gfx.h"
#include "ImGuiBackend.h"
#include "Os.h"

#define ImTextureID Playground::Gfx::DescriptorHandle
#include <imgui/imgui.h>

using namespace Playground;
using namespace Gfx;

struct TriangleShader;

struct TrianglePipeline : public ShaderPipeline<TriangleShader> {
    TrianglePipeline(TriangleShader* owner)
        : ShaderPipeline<TriangleShader>(owner)
    {
    }

    Optional<Box<Pipeline>> Build() override
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = GetDefaultPipelineStateDesc(owner_->device_);
        pso_desc.NumRenderTargets = 1;
        // TODO: query
        pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

        {
            static const char* vertex_shader = "\
				struct VS_INPUT\
				{\
				  float2 pos : POSITION;\
				  float4 col : COLOR0;\
				};\
				\
				struct PS_INPUT\
				{\
				  float4 pos : SV_POSITION;\
				  float4 col : COLOR0;\
				};\
				\
				PS_INPUT main(VS_INPUT input)\
				{\
				  PS_INPUT output;\
				  output.pos = float4(input.pos.xy, 0.f, 1.f);\
				  output.col = input.col;\
				  return output;\
				}";

            pso_desc.VS = GetShaderFromStaticSource({ .source = vertex_shader,
                                                        .entrypoint = L"main",
                                                        .profile = L"vs_6_0" })
                              ->GetBytecode();
        }

        // Create the input layout
        static D3D12_INPUT_ELEMENT_DESC local_layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };
        pso_desc.InputLayout = { local_layout, _countof(local_layout) };

        {
            static const char* pixel_shader = "struct PS_INPUT\
				{\
				  float4 pos : SV_POSITION;\
				  float4 col : COLOR0;\
				};\
				\
				float4 main(PS_INPUT input) : SV_Target\
				{\
				  float4 out_col = input.col; \
				  return out_col; \
				}";

            pso_desc.PS = GetShaderFromStaticSource({ .source = pixel_shader,
                                                        .entrypoint = L"main",
                                                        .profile = L"ps_6_0" })
                              ->GetBytecode();
        }

        return Pipeline::From(owner_->device_, pso_desc);
    }
};

struct TriangleShader : public Gfx::Shader {
    void Init(Gfx::Device* device) override
    {
        Shader::Init(device);
        pipeline_ = MakeBox<TrianglePipeline>(this);
    }
};

int main(int argc, char** argv)
{
    Engine::Start();
    {
        Device device;

        const i32 BACKBUFFERS_NUM = 3;
        const i32 MAX_FRAMES_IN_FLIGHT = 3;

        Swapchain* swapchain = CreateWindowAndSwapchain(&device, { 1920, 1080 }, BACKBUFFERS_NUM);
        Os::Window* window = swapchain->window_;

        ImGuiRenderer imgui_renderer;
        imgui_renderer.Init(&device);

        Array<Gfx::Waitable> frame_waitables;

        Box<TriangleShader> shader = MakeBox<TriangleShader>();
        shader->Init(&device);

        while (window->PumpMessages()) {
            if (frame_waitables.Size() >= MAX_FRAMES_IN_FLIGHT) {
                frame_waitables.RemoveAt(0).Wait();
            }

            Engine::FrameBegin();

            {
                ImGui::Text("Hello world!");
            }

            RenderTargetDesc backbuffer_rt = swapchain->GetCurrentBackbufferAsRenderTarget();
            Gfx::Pass* clear_pass = device.graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                                        .Attach({ .resource = *backbuffer_rt.resource->resource_ }, D3D12_RESOURCE_STATE_RENDER_TARGET));

            Gfx::Pass* present_pass = device.graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                                          .Attach({ .resource = *backbuffer_rt.resource->resource_ }, D3D12_RESOURCE_STATE_PRESENT));

            Encoder encoder = device.CreateEncoder();
            f32 clear_color[4] = { 0.2f, 0.1f, 0.2f, 0.f };
            encoder.SetPass(clear_pass);
            encoder.GetCmdList()->ClearRenderTargetView(device._GetSrcHandle(backbuffer_rt.rtv), clear_color, 0, nullptr);

            {
                struct Vertex {
                    Vector2 position;
                    Color4ub colour;
                };

                Resource vertex_buffer = device.CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, 3 * SizeOf<Vertex>(), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ);
                Vertex* vtx_dst = nullptr;
                verify_hr(vertex_buffer.resource_->Map(0, nullptr, reinterpret_cast<void**>(&vtx_dst)));
                vtx_dst[0] = { .position = { -0.5f, -0.5f }, .colour = { 255, 0, 0, 0 } };
                vtx_dst[1] = { .position = { 0.5f, -0.5f }, .colour = { 0, 255, 0, 0 } };
                vtx_dst[2] = { .position = { 0.f, 0.5f }, .colour = { 0, 0, 255, 0 } };
                vertex_buffer.resource_->Unmap(0, nullptr);

                unsigned int stride = sizeof(Vertex);
                unsigned int offset = 0;
                D3D12_VERTEX_BUFFER_VIEW vbv;
                memset(&vbv, 0, sizeof(D3D12_VERTEX_BUFFER_VIEW));
                vbv.BufferLocation = vertex_buffer.resource_->GetGPUVirtualAddress();
                vbv.SizeInBytes = 3 * stride;
                vbv.StrideInBytes = stride;
                encoder.GetCmdList()->IASetVertexBuffers(0, 1, &vbv);
                encoder.GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = device._GetSrcHandle(backbuffer_rt.rtv);
                encoder.GetCmdList()->OMSetRenderTargets(1, &rtv_handle, false, nullptr);
                D3D12_VIEWPORT vp;
                memset(&vp, 0, sizeof(D3D12_VIEWPORT));
                vp.Width = As<f32>(window->resolution_.x());
                vp.Height = As<f32>(window->resolution_.y());
                vp.MinDepth = 0.0f;
                vp.MaxDepth = 1.0f;
                vp.TopLeftX = vp.TopLeftY = 0.0f;
                encoder.GetCmdList()->RSSetViewports(1, &vp);
                const D3D12_RECT r = { 0, 0, window->resolution_.x(), window->resolution_.y() };
                encoder.GetCmdList()->RSSetScissorRects(1, &r);
                encoder.GetCmdList()->SetPipelineState(shader->pipeline_->GetPSO());
                encoder.GetCmdList()->DrawInstanced(3, 1, 0, 0);

                device.ReleaseWhenCurrentFrameIsDone(std::move(vertex_buffer));
            }

            ImGui::Render();
            imgui_renderer.RenderDrawData(ImGui::GetDrawData(), &encoder, backbuffer_rt);

            encoder.SetPass(present_pass);
            encoder.Submit();

            verify_hr(swapchain->swapchain_->Present(1, 0));
            device.AdvanceFence();
            device.RecycleResources();

            Gfx::Waitable frame_end_fence = device.GetWaitable();
            frame_waitables.PushBack(frame_end_fence);
        }

        device.GetWaitable().Wait();
    }
    Engine::Shutdown();
}