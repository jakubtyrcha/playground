#pragma once

#include "RenderComponent.h"
#include "gfx.h"

namespace Rendering {

struct Pointset {
    struct PointPayload {
        Vector3 position;
        float size;
        Color4ub colour;
    };

    Containers::Array<PointPayload> points_;

    void Add(Vector3 position, float size, Color4ub colour);
};

struct PointsetRenderer : public RenderComponent {
    Core::Box<Gfx::IPipelineBuilder> pipeline_;

    struct FrameData {
        Gfx::Resource upload_buffer_;
        Containers::Array<Gfx::Resource> constant_buffers_;
        Gfx::Waitable waitable_;
    };

    Containers::Array<FrameData> frame_data_queue_;

    Gfx::Pass* update_pass_ = nullptr;
    Gfx::Pass* particle_pass_ = nullptr;

    Gfx::Resource points_buffer_;
    Gfx::Resource* colour_target_ = nullptr;
    Gfx::Resource* depth_buffer_ = nullptr;
    Gfx::Resource* motionvec_target_ = nullptr;

    Pointset* pointset_ = nullptr;

    void Init(Gfx::Device* device) override;
    void AddPassesToGraph() override;
    void Render(Gfx::Encoder* encoder, ViewportRenderContext * viewport_ctx, D3D12_CPU_DESCRIPTOR_HANDLE* rtv_handles, D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle) override;
};

}