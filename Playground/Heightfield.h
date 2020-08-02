#pragma once

#include "RenderComponent.h"
#include "gfx.h"
#include "geometry.h"

namespace Playground {
    
struct HeightfieldRenderer : public RenderComponent {
    Box<Gfx::IPipelineBuilder> pipeline_;

    Gfx::Pass* pass_ = nullptr;

    Gfx::Resource* colour_target_ = nullptr;
    Gfx::Resource* depth_buffer_ = nullptr;
    Gfx::Resource* motionvec_target_ = nullptr;

    Optional<Gfx::Resource> heightfield_texture_;
    Gfx::DescriptorHandle heightfield_srv_;
    Gfx::Resource index_buffer_;
    AABox3D bounding_box_;
    Vector2i resolution_;
    Vector3 light_dir_;
    Optional<Gfx::Resource> horizon_angle_texture_;
    Gfx::DescriptorHandle horizon_angle_srv_;

    void CreateIB();

    void Init(Gfx::Device* device) override;
    void AddPassesToGraph() override;
    void Render(Gfx::Encoder* encoder, ViewportRenderContext const * viewport_ctx, D3D12_CPU_DESCRIPTOR_HANDLE* rtv_handles, D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle) override;
};

}