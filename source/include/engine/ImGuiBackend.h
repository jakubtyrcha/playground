#pragma once
#include "Gfx.h"
#include "Shader.h"

struct ImDrawData;
struct D3D12_VERTEX_BUFFER_VIEW;
struct D3D12_INDEX_BUFFER_VIEW;

namespace Playground {
struct ImGuiShader : public Gfx::Shader {
    void Init(Gfx::Device* device) override;
};

struct ImGuiRenderer {
    Gfx::Device* device_ = nullptr;
    Box<ImGuiShader> shader_;
    Gfx::Resource font_texture_;
    Gfx::DescriptorHandle font_texture_srv_;

    void Init(Gfx::Device* device);

    void RenderDrawData(ImDrawData* draw_data, Gfx::Encoder * encoder, Gfx::RenderTargetDesc rtv);

    void _CreateFontsTexture();
    void _SetupRenderState(ImDrawData* draw_data, Gfx::Encoder* encoder, D3D12_VERTEX_BUFFER_VIEW vbv, D3D12_INDEX_BUFFER_VIEW ibv, Gfx::DescriptorHandle cbv);
};
}