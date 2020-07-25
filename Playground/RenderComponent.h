#pragma once

namespace Gfx {
    struct Device;
    struct Encoder;
}

namespace Rendering {
struct ViewportRenderContext;

struct RenderComponent {
    Gfx::Device* device_ = nullptr;

    virtual ~RenderComponent();

    virtual void Init(Gfx::Device* device);
    virtual void AddPassesToGraph();
    virtual void Render(Gfx::Encoder* encoder, ViewportRenderContext * viewport_ctx, D3D12_CPU_DESCRIPTOR_HANDLE * rtv_handles, D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle);
};

}