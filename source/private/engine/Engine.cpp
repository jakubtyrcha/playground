#include "Pch.h"

#include "Engine.h"
#include "Os.h"
#include "Gfx.h"

#define ImTextureID Playground::Gfx::DescriptorHandle
#include <imgui/imgui.h>
#include "imgui_impl_win32.h"

void InitImgui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
}

namespace Playground {

using namespace Gfx;

namespace Engine {
    void Start()
    {
        InitImgui();
    }

    void Shutdown()
    {
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        Device::ShutdownAllocator();
    }

    void FrameBegin()
    {
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }
}

namespace Gfx {

    Swapchain* CreateWindowAndSwapchain(Device* device, Vector2i resolution, i32 backbuffers)
    {
        Os::Window* window = Os::NewWindow(resolution);
        Gfx::Swapchain* swapchain = device->CreateSwapchain(window, backbuffers);

        window->Init();
        ImGui_ImplWin32_Init(window->hwnd_);

        return swapchain;
    }
}
}