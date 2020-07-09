#include "pch.h"

#include "array.h"
#include "assertions.h"
#include "files.h"
#include "gfx.h"
#include "os.h"
#include <fmt/format.h>

#include "shader.h"

#include "imgui_impl_win32.h"
#include <imgui/imgui.h>

#include "imgui_backend.h"
#include "particles.h"
#include "rendering.h"
#include "shapes.h"

#include <math.h>

using namespace Containers;
using namespace IO;

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

int main(int argc, char** argv)
{
    Gfx::Device device;

    InitImgui();

    Rendering::ImGuiRenderer imgui_renderer;
    imgui_renderer.Init(&device);

    Rendering::ImmediateModeShapeRenderer shape_renderer;
    shape_renderer.Init(&device);

    Rendering::PolygonParticleGenerator particle_generator;
    particle_generator.Init(&device, 100000, 1000.f, 10.f);
    particle_generator.vertices_.PushBack({ -2, 1, -2 });
    particle_generator.vertices_.PushBack({ 2, 1, -2 });
    particle_generator.vertices_.PushBack({ -2, 1, 2 });
    particle_generator.vertices_.PushBack({ 2, 1, -2 });
    particle_generator.vertices_.PushBack({ 2, 1, 2 });
    particle_generator.vertices_.PushBack({ -2, 1, 2 });

    //Rendering::SphereTracer sphere_tracer;
    //sphere_tracer.Init(&device);

    // TODO: who should own backbuffers num?
    // Device, swapchain?
    i32 backbuffers_num = 3;

    // TODO: this should be one call creating (device, window, swapchain) tuple?
    // one device can have many (window, swapchain) pairs
    Os::Window* window = Os::CreateOsWindow({ 1920, 1080 });
    Gfx::Swapchain* window_swapchain = device.CreateSwapchain(window, backbuffers_num);
    // this is here, because WndProc needs the mappings ready
    window->Init();
    ImGui_ImplWin32_Init(window->hwnd_);

    struct ScreenResources {
        Gfx::Resource random_access_texture;
        Gfx::Resource depth_buffer;
        Vector2i resolution;
    };

    ScreenResources screen_resources { .resolution = window->resolution_ };
    screen_resources.random_access_texture = device.CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, window->resolution_, DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_RENDER_TARGET);
    screen_resources.depth_buffer = device.CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, window->resolution_, DXGI_FORMAT_D24_UNORM_S8_UINT, 1, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_DEPTH_WRITE);

    int frames_ctr = 3;

    Array<Gfx::Waitable> frame_waitables;

    Rendering::Viewport main_viewport {};

    main_viewport.camera_look_at = { 0, 0, 0 };
    main_viewport.camera_position = { 5, 5, -5 };
    main_viewport.camera_up = { 0, 1, 0 };
    main_viewport.resolution = window->resolution_;
    main_viewport.fov_y = Magnum::Math::Constants<float>::piHalf();
    main_viewport.near_plane = 0.1f;
    main_viewport.far_plane = 1000.f;

    window->RunMessageLoop([&window,
                               &device,
                               &window_swapchain,
                               &screen_resources,
                               backbuffers_num,
                               &frames_ctr,
                               &frame_waitables,
                               &imgui_renderer,
                               &shape_renderer,
                               &main_viewport,
                               &particle_generator]() {
        i32 current_backbuffer_index = window_swapchain->swapchain_->GetCurrentBackBufferIndex();

        if (screen_resources.resolution != window->resolution_) {
            screen_resources.random_access_texture = device.CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, window->resolution_, DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            screen_resources.depth_buffer = device.CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, window->resolution_, DXGI_FORMAT_R24G8_TYPELESS, 1, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_DEPTH_WRITE);
            screen_resources.resolution = window->resolution_;
            main_viewport.resolution = window->resolution_;
        }

        f32 camera_sensivity = 0.1f;
        if (window->user_input_.keys_down_['W']) {
            main_viewport.TranslateCamera(main_viewport.GetLookToVector() * camera_sensivity);
        }
        if (window->user_input_.keys_down_['S']) {
            main_viewport.TranslateCamera(-main_viewport.GetLookToVector() * camera_sensivity);
        }
        if (window->user_input_.keys_down_['A']) {
            main_viewport.TranslateCamera(-main_viewport.GetRightVector() * camera_sensivity);
        }
        if (window->user_input_.keys_down_['D']) {
            main_viewport.TranslateCamera(main_viewport.GetRightVector() * camera_sensivity);
        }

        if (window->user_input_.keys_down_['R'] && window->user_input_.keys_down_[0x10]) {
            while (frame_waitables.Size()) {
                frame_waitables.RemoveAt(0).Wait();
            }

            Gfx::ReloadShaders();
        }

        ImGuiIO& io = ImGui::GetIO();

        if (!io.WantCaptureMouse && window->user_input_.mouse_down_[0]) {
            f32 delta_x = io.MouseDelta.x / static_cast<f32>(window->resolution_.x());
            f32 delta_y = -io.MouseDelta.y / static_cast<f32>(window->resolution_.y());

            //
            Vector4 clip_space_point_to { delta_x * 2.f, delta_y * 2.f, 1.f, 1.f };

            Vector4 world_point_to = main_viewport.inv_view_projection_matrix * clip_space_point_to;

            main_viewport.camera_look_at = main_viewport.camera_position + world_point_to.xyz().normalized();
        }

        particle_generator.Tick(io.DeltaTime);

        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        main_viewport.view_matrix = Rendering::LookAtLh(main_viewport.camera_look_at, main_viewport.camera_position, main_viewport.camera_up);
        main_viewport.inv_view_matrix = Rendering::InverseLookAtLh(main_viewport.camera_look_at, main_viewport.camera_position, main_viewport.camera_up);
        main_viewport.projection_matrix = Rendering::PerspectiveFovLh(main_viewport.GetAspectRatio(), main_viewport.fov_y, main_viewport.near_plane, main_viewport.far_plane);
        main_viewport.inv_projection_matrix = Rendering::InversePerspectiveFovLh(main_viewport.GetAspectRatio(), main_viewport.fov_y, main_viewport.near_plane, main_viewport.far_plane);
        main_viewport.view_projection_matrix = main_viewport.projection_matrix * main_viewport.view_matrix;
        main_viewport.inv_view_projection_matrix = main_viewport.inv_view_matrix * main_viewport.inv_projection_matrix;

        Rendering::ShapeVertex vertices[] = {
            { .position = { 0, 0, 0 }, .colour = { 255, 0, 0, 1 } },
            { .position = { 1, 0, 0 }, .colour = { 255, 0, 0, 1 } },
            { .position = { 0, 0, 0 }, .colour = { 0, 255, 0, 1 } },
            { .position = { 0, 1, 0 }, .colour = { 0, 255, 0, 1 } },
            { .position = { 0, 0, 0 }, .colour = { 0, 0, 255, 1 } },
            { .position = { 0, 0, 1 }, .colour = { 0, 0, 255, 1 } },
        };
        shape_renderer.vertices_.Append(vertices, _countof(vertices));

        {
            ImGui::Begin("Particles");

            ImGui::Text("Time: %f ms", io.DeltaTime * 1000.f);
            ImGui::Text("Particles num: %d", particle_generator.NumParticles());
            ImGui::Text("Particles pages: %d", particle_generator.active_pages_.Size());

            ImGui::End();
        }

        ID3D12Resource* current_backbuffer = *window_swapchain->backbuffers_[current_backbuffer_index];

        Gfx::Pass* clear_bb_pass = device.graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                                       .Attach({ .resource = *screen_resources.random_access_texture.resource_ }, D3D12_RESOURCE_STATE_RENDER_TARGET)
                                                                       .Attach({ .resource = *screen_resources.depth_buffer.resource_ }, D3D12_RESOURCE_STATE_DEPTH_WRITE));

        particle_generator.AddPassesToGraph(&screen_resources.random_access_texture, &screen_resources.depth_buffer);

        Gfx::Pass* render_ui_pass = device.graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                                        .Attach({ .resource = *screen_resources.random_access_texture.resource_ }, D3D12_RESOURCE_STATE_RENDER_TARGET));

        Gfx::Pass* copy_to_backbuffer_pass = device.graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                                                 .Attach({ .resource = *screen_resources.random_access_texture.resource_ }, D3D12_RESOURCE_STATE_COPY_SOURCE)
                                                                                 .Attach({ .resource = current_backbuffer }, D3D12_RESOURCE_STATE_COPY_DEST));

        Gfx::Pass* present_pass = device.graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                                      .Attach({ .resource = current_backbuffer }, D3D12_RESOURCE_STATE_PRESENT));

        Gfx::Encoder encoder = device.CreateEncoder();

        ImGui::Render();

        encoder.SetPass(clear_bb_pass);

        D3D12_RENDER_TARGET_VIEW_DESC rtv_desc {
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
            .Texture2D = {}
        };
        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = device.rtvs_descriptor_heap_.heap_->GetCPUDescriptorHandleForHeapStart();
        rtv_handle.ptr += device.rtvs_descriptor_heap_.AllocateTable(1) * device.rtvs_descriptor_heap_.increment_;
        device.device_->CreateRenderTargetView(*screen_resources.random_access_texture.resource_, &rtv_desc, rtv_handle);

        f32 clear_colour[4] = {};
        encoder.GetCmdList()->ClearRenderTargetView(rtv_handle, clear_colour, 0, nullptr);

        D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc {
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
            .Flags = D3D12_DSV_FLAG_NONE,
            .Texture2D = {}
        };
        D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle = device.dsvs_descriptor_heap_.heap_->GetCPUDescriptorHandleForHeapStart();
        dsv_handle.ptr += device.dsvs_descriptor_heap_.AllocateTable(1) * device.dsvs_descriptor_heap_.increment_;
        device.device_->CreateDepthStencilView(*screen_resources.depth_buffer.resource_, &dsv_desc, dsv_handle);

        encoder.GetCmdList()->ClearDepthStencilView(dsv_handle, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);

        //sphere_tracer.Render(&encoder, &main_viewport, rtv_handle);
        particle_generator.Render(&encoder, &main_viewport, rtv_handle, dsv_handle);
        encoder.SetPass(render_ui_pass);
        shape_renderer.Render(&encoder, &main_viewport, rtv_handle);
        imgui_renderer.RenderDrawData(ImGui::GetDrawData(), &encoder, rtv_handle);

        encoder.SetPass(copy_to_backbuffer_pass);
        encoder.GetCmdList()->CopyResource(current_backbuffer, *screen_resources.random_access_texture.resource_);

        encoder.SetPass(present_pass);

        // we should wait for the presenting being done before we access that backbuffer again?
        if (frame_waitables.Size() == backbuffers_num) {
            frame_waitables.RemoveAt(0).Wait();
        }

        encoder.Submit();

        verify_hr(window_swapchain->swapchain_->Present(1, 0));
        device.AdvanceFence();
        device.RecycleResources();

        frame_waitables.PushBack(device.GetWaitable());

        current_backbuffer_index = (current_backbuffer_index + 1) % backbuffers_num;
    });

    device.GetWaitable().Wait();

    ImGui_ImplWin32_Shutdown();
    imgui_renderer.Shutdown();
    shape_renderer.Shutdown();
    ImGui::DestroyContext();

    return 0;
}
