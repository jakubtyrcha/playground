#include "Pch.h"
#include "Gfx.h"
#include "Os.h"
#include "Engine.h"

using namespace Playground;
using namespace Gfx;

int main(int argc, char** argv)
{
    Engine::Start();
    Device device;

    const i32 BACKBUFFERS_NUM = 3;
    const i32 MAX_FRAMES_IN_FLIGHT = 3;

    Swapchain * swapchain = CreateWindowAndSwapchain(&device, { 1920, 1080 }, BACKBUFFERS_NUM);
    Os::Window* window = swapchain->window_;

    Array<Gfx::Waitable> frame_waitables;

    while (window->PumpMessages()) {
        if (frame_waitables.Size() >= MAX_FRAMES_IN_FLIGHT) {
            frame_waitables.RemoveAt(0).Wait();
        }

        i32 current_backbuffer_index = swapchain->swapchain_->GetCurrentBackBufferIndex();

        Engine::FrameBegin();

        verify_hr(swapchain->swapchain_->Present(1, 0));
        device.AdvanceFence();
        device.RecycleResources();

        Gfx::Waitable frame_end_fence = device.GetWaitable();
        frame_waitables.PushBack(frame_end_fence);
    }

    device.GetWaitable().Wait();

    Engine::Shutdown();
}