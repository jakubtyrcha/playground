#pragma once

#include "gfx.h"

namespace Playground {

struct ViewportRenderContext;

// TODO: bad name
template <typename RenderComponent>
struct RenderComponentPipeline : public Gfx::IPipelineBuilder {
    RenderComponent* owner_ = nullptr;

    RenderComponentPipeline(RenderComponent* render_component)
        : owner_(render_component)
    {
    }
};

// TODO: split on component and instance (fire-forget instance)
struct RenderComponent {
    Gfx::Device* device_ = nullptr;

    virtual ~RenderComponent();

    virtual void Init(Gfx::Device* device);
    virtual void AddPassesToGraph();
    virtual void Render(Gfx::Encoder* encoder, ViewportRenderContext* viewport_ctx, D3D12_CPU_DESCRIPTOR_HANDLE* rtv_handles, D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle);
};

}