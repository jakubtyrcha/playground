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

    Gfx::Pass* taa_pass_ = nullptr;
    Gfx::Resource* colour_src_ = nullptr;
    Gfx::Resource* depth_src_ = nullptr;
    Gfx::Resource* prev_colour_src_ = nullptr;

    struct FrameData {
        Containers::Array<Gfx::Resource> constant_buffers_;
        Gfx::Waitable waitable_;
    };

    Containers::Array<FrameData> frame_data_queue_;

    void Init(Gfx::Device* device);
    // TODO: add proxy Gfx::Resource for bb
    void AddPassesToGraph(ID3D12Resource* colour_target, Gfx::Resource* colour_src, Gfx::Resource* depth_src, Gfx::Resource* prev_colour_src);
    void Render(Gfx::Encoder* encoder, Viewport* viewport, D3D12_CPU_DESCRIPTOR_HANDLE colour_target_handle);
};
}
