#include "pch.h"
#include "RenderComponent.h"

namespace Playground {
RenderComponent::~RenderComponent() {}

void RenderComponent::Init(Gfx::Device* device)
{
    device_ = device;
}

void RenderComponent::AddPassesToGraph()
{
}

void RenderComponent::Render(Gfx::Encoder* encoder, ViewportRenderContext * viewport_ctx, D3D12_CPU_DESCRIPTOR_HANDLE * rtv_handles, D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle)
{
}

}