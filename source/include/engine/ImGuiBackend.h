#include "Gfx.h"
#include "Shader.h"

namespace Playground {
struct ImGuiShader : public Gfx::Shader {
    void Init(Gfx::Device* device) override;
};

struct ImGuiRenderer {
    Gfx::Device* owner_ = nullptr;
    Box<ImGuiShader> shader_;
    Gfx::Resource font_texture_;
    Gfx::DescriptorHandle font_texture_srv_;

    void Init(Gfx::Device* device);

    void _CreateFontsTexture();
};
}