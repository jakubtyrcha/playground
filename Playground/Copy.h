#pragma once

#include "gfx.h"
#include "random.h"
#include "shader.h"
#include "box.h"

namespace Playground {
struct Viewport;

struct Copy {
    Gfx::Device* device_ = nullptr;

    Box<Gfx::IPipelineBuilder> pipeline_;

    Gfx::Pass* pass_ = nullptr;
    Gfx::Resource* colour_src_ = nullptr;

    struct FrameData {
        Array<Gfx::Resource> constant_buffers_;
        Gfx::Waitable waitable_;
    };

    Array<FrameData> frame_data_queue_;

    void Init(Gfx::Device* device);
    void AddPassesToGraph(ID3D12Resource* colour_target, Gfx::Resource* colour_src);
    void Render(Gfx::Encoder* encoder, Viewport* viewport, D3D12_CPU_DESCRIPTOR_HANDLE colour_target_handle);
};

}