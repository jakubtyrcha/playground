#pragma once

#include "RenderComponent.h"
#include "gfx.h"

namespace Playground {

struct Pointset {
    struct PointPayload {
        Vector3 position;
        float size;
        Vector3 prev_position;
        Color4 colour;
    };

    Array<PointPayload> points_;
    bool dirty_ = false;

    void Add(Vector3 position, float size, Color4 colour);
    i64 Size() const;
};

struct PointsetRenderer : public RenderComponent {
    Box<Gfx::IPipelineBuilder> pipeline_;

    Gfx::Pass* update_pass_ = nullptr;
    Gfx::Pass* particle_pass_ = nullptr;

    u32 current_capacity_ = 0;
    Optional<Gfx::Resource> points_buffer_;
    Gfx::DescriptorHandle points_buffer_srv_;
    Gfx::Resource* colour_target_ = nullptr;
    Gfx::Resource* depth_buffer_ = nullptr;
    Gfx::Resource* motionvec_target_ = nullptr;

    Pointset* pointset_ = nullptr;

    void Init(Gfx::Device* device) override;
    void AddPassesToGraph() override;
    void Render(Gfx::Encoder* encoder, ViewportRenderContext* viewport_ctx, D3D12_CPU_DESCRIPTOR_HANDLE* rtv_handles, D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle) override;
};

}