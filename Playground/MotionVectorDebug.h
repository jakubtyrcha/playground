#pragma once

#include "gfx.h"
#include "box.h"
#include "RenderComponent.h"

namespace Playground {

struct MotionVectorDebug {
    Gfx::Device* device_ = nullptr;

    Box<Gfx::IPipelineBuilder> pipeline_;

    Gfx::Pass* pass_ = nullptr;

    Gfx::Resource* motion_vector_src_ = nullptr;

    void Init(Gfx::Device* device);
    void AddPassesToGraph(ID3D12Resource* colour_target, Gfx::Resource* motion_vector_src);
    void Render(Gfx::Encoder* encoder, ViewportRenderContext* viewport_ctx, D3D12_CPU_DESCRIPTOR_HANDLE colour_target_handle);
};

}