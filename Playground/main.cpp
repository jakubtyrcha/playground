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

#include "Copy.h"
#include "MotionVectorDebug.h"
#include "Pointset.h"
#include "Rendering.h"
#include "Shapes.h"
#include "Taa.h"
#include "imgui_backend.h"
#include <fastnoise/FastNoise.h>

#include <math.h>

using namespace Containers;
using namespace Algorithms;
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

namespace Rendering {

enum class TAAPattern {
    Standard4x,
    Standard8x
};

void SetPattern(Rendering::Viewport& viewport, TAAPattern pattern)
{
    viewport.taa_offsets.Clear();

    switch (pattern) {
    case TAAPattern::Standard4x: {
        // msaa 4x
        viewport.taa_offsets.PushBack({ -2, -6 });
        viewport.taa_offsets.PushBack({ 6, -2 });
        viewport.taa_offsets.PushBack({ -6, 2 });
        viewport.taa_offsets.PushBack({ 2, 6 });
    } break;
    case TAAPattern::Standard8x: {
        // msaa 8x
        viewport.taa_offsets.PushBack({ 1, -3 });
        viewport.taa_offsets.PushBack({ -1, 3 });
        viewport.taa_offsets.PushBack({ 5, 1 });
        viewport.taa_offsets.PushBack({ -3, -5 });
        viewport.taa_offsets.PushBack({ -5, 5 });
        viewport.taa_offsets.PushBack({ -7, -1 });
        viewport.taa_offsets.PushBack({ 3, 7 });
        viewport.taa_offsets.PushBack({ 7, -7 });
    } break;
    }

    for (Vector2& sample : viewport.taa_offsets) {
        sample /= 8.f;
    }

    viewport.taa_index = 0;
}
}



Containers::Array<Vector2> Generate2DGridSamples(i32 N, Vector2 v0, Vector2 v1)
{
    Containers::Array<Vector2> output;
    output.Reserve(N);

    Random::Generator gen {};
    
    for (i32 i = 0; i < N; i++) {
        output.PushBack({ gen.F32UniformInRange(v0.x(), v1.x()), gen.F32UniformInRange(v0.y(), v1.y()) });
    }

    return output;
}

Vector2 RandomPointInAnnulus(f32 r0, f32 r1, Vector2 z) {
    assert(r0 < r1);

    f32 theta = 2.f * Magnum::Math::Constants<f32>::pi() * z.x();
    f32 d = sqrtf(z.y() * (r1 * r1 - r0 * r0) + r0 * r0);

    return d * Vector2{ cosf(theta), sinf(theta) };
}

// Robert Bridson algorithm
Containers::Array<Vector2> Generate2DGridSamplesPoissonDisk(Vector2 v0, Vector2 v1, float min_distance, i32 samples_before_rejection)
{
    Containers::Array<Vector2> output;
    Random::Generator gen {};
    
    constexpr i32 DIM = 2;
    const i32 k = samples_before_rejection;
    const f32 r = min_distance;
    const f32 r2 = r * r;

    const f32 cell_size = r / sqrtf(DIM);

    const Vector2 span = v1 - v0;
    const Vector2i grid_cells = Vector2i { Math::ceil((span / cell_size)) };
    const f32 inv_cell_size = 1.f / cell_size;

    Containers::Array<i32> grid;
    grid.ResizeUninitialised(grid_cells.x() * grid_cells.y());

    output.Reserve(grid.Size());

    for(i32 i=0; i < grid_cells.x() * grid_cells.y(); i++) {
        grid[i] = -1;
    }

    Vector2 x0 { gen.F32UniformInRange(0.f, span.x()), gen.F32UniformInRange(0.f, span.y()) };
    Vector2i grid_coord = Vector2i { Math::floor(x0 * inv_cell_size) };

    output.PushBack(x0);
    grid[grid_coord.x() + grid_coord.y() * grid_cells.x()] = 0;

    Containers::Array<i32> active_list;
    active_list.PushBack(0);
    
    while (active_list.Size()) {
        i32 index = gen.I32UniformInRange(0, static_cast<i32>( active_list.Size()));
        Vector2 xi = output[active_list[index]];

        int tries = k;
        while(tries) {
            Vector2 offset = RandomPointInAnnulus(r, 2.f * r, { gen.F32Uniform(), gen.F32Uniform() });
            Vector2 s = xi + offset;

            bool rejected = false;
            Vector2i grid_coord = Vector2i { Math::floor(s * inv_cell_size) };

            assert(grid_coord != Vector2i { Math::floor(xi * inv_cell_size) });

            if(((s < Vector2{0.f}) || (span <= s)).any() ) {
                rejected = true;
                goto break_double_loop;
            }

            assert((Vector2i { 0, 0 } <= grid_coord && grid_coord < grid_cells).all());

            for (i32 x = Max(0, grid_coord.x() - 2); x < Min(grid_cells.x(), grid_coord.x() + 3); x++) {
                for (i32 y = Max(0, grid_coord.y() - 2); y < Min(grid_cells.y(), grid_coord.y() + 3); y++) {
                    i32 tested_index = x + y * grid_cells.x();

                    if(grid[tested_index] == -1) {
                        continue;
                    }
                    
                    if((s - output[grid[tested_index]]).dot() < r2) {
                        rejected = true;
                        goto break_double_loop;
                    }
                }
            }

        break_double_loop:
            if(rejected) {
                tries--;
                continue;
            }
            
            i32 next_sample_index = static_cast<i32>(output.Size());
            assert(grid[grid_coord.x() + grid_coord.y() * grid_cells.x()] == -1);
            grid[grid_coord.x() + grid_coord.y() * grid_cells.x()] = next_sample_index;
            active_list.PushBack(next_sample_index);
            output.PushBack(s);
            assert(tries);
            break;
        }

        if(tries == 0) {
            active_list.RemoveAtAndSwapWithLast(index);
        }
    }

    for(Vector2 & p : output) {
        p += v0;
    }

    return output;
}

int main(int argc, char** argv)
{
    Gfx::Device device;

    InitImgui();

    Rendering::ImGuiRenderer imgui_renderer;
    imgui_renderer.Init(&device);

    Rendering::ImmediateModeShapeRenderer shape_renderer;
    shape_renderer.Init(&device);

    Rendering::Pointset pointset;

    i32 N = 1000;
    Vector2 v0 { -5.f, -5.f };
    Vector2 v1 { 5.f, 5.f };

    f32 span = Min((v1-v0).x(), (v1-v0).y());
    f32 pointsize = 0.125f * span / sqrtf(static_cast<f32>( N ));
    //for (Vector2 p : Generate2DGridSamples(N, v0, v1)) 
    FastNoise noise_gen;
    noise_gen.SetNoiseType(FastNoise::Simplex);

    for (Vector2 p : Generate2DGridSamplesPoissonDisk(v0, v1, pointsize * 5.f, 30)) 
    {
        Vector2 c = ((p - v0) / (v1 - v0));
        pointset.Add({ p.x(), noise_gen.GetNoise(10 * p.x(), 10 * p.y()), p.y() }, pointsize, Color4 { c.x(), c.y(), 0.05f, 1.f });
    }

    Rendering::PointsetRenderer pointset_renderer;
    pointset_renderer.Init(&device);

    pointset_renderer.pointset_ = &pointset;

    Rendering::TAA taa;
    taa.Init(&device);

    Rendering::MotionVectorDebug motion_debug;
    motion_debug.Init(&device);

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

    Containers::Array<Rendering::FrameData> frame_data_queue;

    i32 max_frames_queued_ = 3;

    struct ScreenResources {
        Gfx::Device* device = nullptr;
        Gfx::Resource final_texture;
        Gfx::Resource colour_texture;
        Gfx::Resource prev_colour_texture;
        Gfx::Resource motion_vectors_texture;
        Gfx::Resource depth_buffer;
        Vector2i resolution;

        DXGI_FORMAT colour_texture_format = DXGI_FORMAT_R16G16B16A16_FLOAT;

        void Resize(Vector2i in_resolution)
        {
            if (resolution == in_resolution) {
                return;
            }

            resolution = in_resolution;

            final_texture = device->CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, resolution, colour_texture_format, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_RENDER_TARGET);
            colour_texture = device->CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, resolution, colour_texture_format, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_RENDER_TARGET);
            prev_colour_texture = device->CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, resolution, colour_texture_format, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_RENDER_TARGET);
            motion_vectors_texture = device->CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, resolution, DXGI_FORMAT_R16G16_FLOAT, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_RENDER_TARGET);
            depth_buffer = device->CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, resolution, DXGI_FORMAT_D24_UNORM_S8_UINT, 1, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        }
    };

    ScreenResources screen_resources { .device = &device };
    screen_resources.Resize(window->resolution_);

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

    i32 taa_pattern = 0;
    SetPattern(main_viewport, static_cast<Rendering::TAAPattern>(taa_pattern));

    window->RunMessageLoop([&window,
                               &device,
                               &window_swapchain,
                               &screen_resources,
                               backbuffers_num,
                               &frames_ctr,
                               &frame_data_queue,
                               &frame_waitables,
                               &imgui_renderer,
                               &shape_renderer,
                               &taa_pattern,
                               &taa,
                               &motion_debug,
                               &main_viewport,
                               &pointset,
                               &pointset_renderer]() {
        i32 current_backbuffer_index = window_swapchain->swapchain_->GetCurrentBackBufferIndex();

        screen_resources.Resize(window->resolution_);
        main_viewport.resolution = window->resolution_;

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

            Vector4 world_point_to = main_viewport.GetInvViewProjectionMatrix() * clip_space_point_to;

            main_viewport.camera_look_at = main_viewport.camera_position + world_point_to.xyz().normalized();
        }

        static bool show_motionvectors = false;
        static bool show_axes_helper = true;

        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        static i32 override_taa_sample = -1;
        if (override_taa_sample != -1) {
            main_viewport.taa_index = override_taa_sample;
        }

        Rendering::ViewportRenderContext viewport_render_context = Rendering::BuildViewportRenderContext(&device, &main_viewport);

        if (show_axes_helper) {
            Rendering::ShapeVertex vertices[] = {
                { .position = { 0, 0, 0 }, .colour = { 255, 0, 0, 1 } },
                { .position = { 1, 0, 0 }, .colour = { 255, 0, 0, 1 } },
                { .position = { 0, 0, 0 }, .colour = { 0, 255, 0, 1 } },
                { .position = { 0, 1, 0 }, .colour = { 0, 255, 0, 1 } },
                { .position = { 0, 0, 0 }, .colour = { 0, 0, 255, 1 } },
                { .position = { 0, 0, 1 }, .colour = { 0, 0, 255, 1 } },
            };
            shape_renderer.vertices_.Append(vertices, _countof(vertices));
        }

        {
            ImGui::Begin("Perf");
            ImGui::Text("Time: %f ms", io.DeltaTime * 1000.f);
            ImGui::End();

            ImGui::Begin("Controls");

            if (ImGui::CollapsingHeader("View")) {
                ImGui::SliderFloat("Near plane", &main_viewport.near_plane, 0.0001f, 1.f);
                ImGui::SliderFloat("TAA decay", &main_viewport.history_decay, 0.f, 1.f);
                ImGui::SliderInt("TAA sample", &override_taa_sample, -1, static_cast<i32>(main_viewport.taa_offsets.Size()) - 1);
                const char* items[] = { "MSAA 4x", "MSAA 8x" };
                if (ImGui::Combo("TAA pattern", &taa_pattern, items, _countof(items))) {
                    SetPattern(main_viewport, static_cast<Rendering::TAAPattern>(taa_pattern));
                }
            }

            if (ImGui::CollapsingHeader("Debug")) {
                ImGui::Checkbox("Axes", &show_axes_helper);
                ImGui::Checkbox("Motion vectors", &show_motionvectors);
            }

            ImGui::End();
        }

        ID3D12Resource* current_backbuffer = *window_swapchain->backbuffers_[current_backbuffer_index];

        Gfx::Pass* clear_pass = device.graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                                    .Attach({ .resource = *screen_resources.colour_texture.resource_ }, D3D12_RESOURCE_STATE_RENDER_TARGET)
                                                                    .Attach({ .resource = *screen_resources.motion_vectors_texture.resource_ }, D3D12_RESOURCE_STATE_RENDER_TARGET)
                                                                    .Attach({ .resource = *screen_resources.depth_buffer.resource_ }, D3D12_RESOURCE_STATE_DEPTH_WRITE));

        pointset_renderer.colour_target_ = &screen_resources.colour_texture;
        pointset_renderer.depth_buffer_ = &screen_resources.depth_buffer;
        pointset_renderer.motionvec_target_ = &screen_resources.motion_vectors_texture;
        pointset_renderer.AddPassesToGraph();

        taa.AddPassesToGraph(&screen_resources.final_texture, current_backbuffer, &screen_resources.colour_texture, &screen_resources.depth_buffer, &screen_resources.prev_colour_texture, &screen_resources.motion_vectors_texture);

        //copy.AddPassesToGraph(current_backbuffer, &screen_resources.final_texture);

        if (show_motionvectors) {
            motion_debug.AddPassesToGraph(current_backbuffer, &screen_resources.motion_vectors_texture);
        }

        Gfx::Pass* render_ui_pass = device.graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                                        .Attach({ .resource = current_backbuffer }, D3D12_RESOURCE_STATE_RENDER_TARGET));

        Gfx::Pass* present_pass = device.graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                                      .Attach({ .resource = current_backbuffer }, D3D12_RESOURCE_STATE_PRESENT));

        Gfx::Encoder encoder = device.CreateEncoder();

        ImGui::Render();

        encoder.SetPass(clear_pass);

        auto create_rtv_handle = [&device](Gfx::Resource* texture, DXGI_FORMAT fmt) -> D3D12_CPU_DESCRIPTOR_HANDLE {
            D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = device.rtvs_descriptor_heap_.heap_->GetCPUDescriptorHandleForHeapStart();
            D3D12_RENDER_TARGET_VIEW_DESC rtv_desc {
                .Format = fmt,
                .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
                .Texture2D = {}
            };
            rtv_handle.ptr += device.rtvs_descriptor_heap_.AllocateTable(1) * device.rtvs_descriptor_heap_.increment_;
            device.device_->CreateRenderTargetView(*texture->resource_, &rtv_desc, rtv_handle);
            return rtv_handle;
        };

        auto create_rtv_handle_ = [&device](ID3D12Resource* texture, DXGI_FORMAT fmt) -> D3D12_CPU_DESCRIPTOR_HANDLE {
            D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = device.rtvs_descriptor_heap_.heap_->GetCPUDescriptorHandleForHeapStart();
            D3D12_RENDER_TARGET_VIEW_DESC rtv_desc {
                .Format = fmt,
                .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
                .Texture2D = {}
            };
            rtv_handle.ptr += device.rtvs_descriptor_heap_.AllocateTable(1) * device.rtvs_descriptor_heap_.increment_;
            device.device_->CreateRenderTargetView(texture, &rtv_desc, rtv_handle);
            return rtv_handle;
        };

        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = create_rtv_handle(&screen_resources.colour_texture, DXGI_FORMAT_R16G16B16A16_FLOAT);
        D3D12_CPU_DESCRIPTOR_HANDLE mv_rtv_handle = create_rtv_handle(&screen_resources.motion_vectors_texture, DXGI_FORMAT_R16G16_FLOAT);

        f32 clear_colour[4] = {};
        encoder.GetCmdList()->ClearRenderTargetView(rtv_handle, clear_colour, 0, nullptr);
        encoder.GetCmdList()->ClearRenderTargetView(mv_rtv_handle, clear_colour, 0, nullptr);

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
        //particle_generator.Render(&encoder, &main_viewport, rtv_handle, dsv_handle, mv_rtv_handle);
        D3D12_CPU_DESCRIPTOR_HANDLE handles[] = { rtv_handle, mv_rtv_handle };
        pointset_renderer.Render(&encoder, &viewport_render_context, handles, dsv_handle);

        shape_renderer.Render(&encoder, &viewport_render_context, rtv_handle, mv_rtv_handle);

        D3D12_CPU_DESCRIPTOR_HANDLE final_rtv_handle = create_rtv_handle(&screen_resources.final_texture, DXGI_FORMAT_R16G16B16A16_FLOAT);

        D3D12_CPU_DESCRIPTOR_HANDLE bb_rtv_handle = create_rtv_handle_(current_backbuffer, DXGI_FORMAT_R8G8B8A8_UNORM);

        taa.Render(&encoder, &viewport_render_context, final_rtv_handle, bb_rtv_handle);

        //copy.Render(&encoder, &main_viewport, bb_rtv_handle);

        if (show_motionvectors) {
            motion_debug.Render(&encoder, &viewport_render_context, bb_rtv_handle);
        }

        encoder.SetPass(render_ui_pass);

        imgui_renderer.RenderDrawData(ImGui::GetDrawData(), &encoder, bb_rtv_handle);

        encoder.SetPass(present_pass);

        // we should wait for the presenting being done before we access that backbuffer again?
        if (frame_waitables.Size() == backbuffers_num) {
            frame_waitables.RemoveAt(0).Wait();
        }

        encoder.Submit();

        verify_hr(window_swapchain->swapchain_->Present(1, 0));
        device.AdvanceFence();
        device.RecycleResources();

        Gfx::Waitable frame_end_fence = device.GetWaitable();
        frame_waitables.PushBack(frame_end_fence);
        viewport_render_context.frame_payload.waitable_ = frame_end_fence;
        frame_data_queue.PushBackRvalueRef(std::move(viewport_render_context.frame_payload));
        current_backbuffer_index = (current_backbuffer_index + 1) % backbuffers_num;

        std::swap(screen_resources.final_texture, screen_resources.prev_colour_texture);
    });

    device.GetWaitable().Wait();

    ImGui_ImplWin32_Shutdown();
    imgui_renderer.Shutdown();
    shape_renderer.Shutdown();
    ImGui::DestroyContext();

    return 0;
}
