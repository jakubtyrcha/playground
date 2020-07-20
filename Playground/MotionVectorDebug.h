#pragma once

#include "gfx.h"
#include "random.h"
#include "shader.h"
#include "box.h"

namespace Rendering {
struct Viewport;

struct MotionVectorDebug {
    Gfx::Device* device_ = nullptr;

    Core::Box<Gfx::IPipelineBuilder> pipeline_;

    Gfx::Pass* pass_ = nullptr;

    Gfx::Resource* motion_vector_src_ = nullptr;

    struct FrameData {
        Containers::Array<Gfx::Resource> constant_buffers_;
        Gfx::Waitable waitable_;
    };

    Containers::Array<FrameData> frame_data_queue_;

    void Init(Gfx::Device* device);
    void AddPassesToGraph(ID3D12Resource* colour_target, Gfx::Resource* motion_vector_src);
    void Render(Gfx::Encoder* encoder, Viewport* viewport, D3D12_CPU_DESCRIPTOR_HANDLE colour_target_handle);
};
}

