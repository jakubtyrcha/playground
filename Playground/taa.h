#pragma once

#include "gfx.h"
#include "random.h"
#include "shader.h"
#include "box.h"

namespace Rendering {
struct Viewport;

struct TAA {
    Gfx::Device* device_ = nullptr;

    Core::Box<Gfx::IPipelineBuilder> pipeline_;

    Gfx::Pass* pass_ = nullptr;
    Gfx::Resource* colour_src_ = nullptr;
    Gfx::Resource* depth_src_ = nullptr;
    Gfx::Resource* prev_colour_src_ = nullptr;
    Gfx::Resource* motion_vector_src_ = nullptr;

    void Init(Gfx::Device* device);
    void AddPassesToGraph(Gfx::Resource* colour_copy_target, ID3D12Resource* colour_target, Gfx::Resource* colour_src, Gfx::Resource* depth_src, Gfx::Resource* prev_colour_src, Gfx::Resource* motion_vector_src);
    void Render(Gfx::Encoder* encoder, ViewportRenderContext* viewport_ctx, D3D12_CPU_DESCRIPTOR_HANDLE colour_copy_target_handle, D3D12_CPU_DESCRIPTOR_HANDLE colour_target_handle);
};
}
