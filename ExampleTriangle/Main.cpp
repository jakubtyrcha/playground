#include "Pch.h"
#include "Engine.h"
#include "Gfx.h"
#include "Os.h"
#include "ImGuiBackend.h"

#define ImTextureID Playground::Gfx::DescriptorHandle
#include <imgui/imgui.h>

using namespace Playground;
using namespace Gfx;

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

        while (window->PumpMessages()) {
            if (frame_waitables.Size() >= MAX_FRAMES_IN_FLIGHT) {
                frame_waitables.RemoveAt(0).Wait();
            }

            Engine::FrameBegin();

            RenderTargetDesc backbuffer_rt = swapchain->GetCurrentBackbufferAsRenderTarget();
            Gfx::Pass* clear_pass = device.graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                                      .Attach({ .resource = *backbuffer_rt.resource->resource_ }, D3D12_RESOURCE_STATE_RENDER_TARGET));

            Gfx::Pass* present_pass = device.graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                                      .Attach({ .resource = *backbuffer_rt.resource->resource_ }, D3D12_RESOURCE_STATE_PRESENT));

            Encoder encoder = device.CreateEncoder();
            f32 clear_color[4] = { 0.1f, 0.2f, 0.1f, 0.f };
            encoder.SetPass(clear_pass);
            encoder.GetCmdList()->ClearRenderTargetView(device._GetSrcHandle(backbuffer_rt.rtv), clear_color, 0, nullptr);

            ImGui::Render();

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