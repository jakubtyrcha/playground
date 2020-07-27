#pragma once

#include "gfx.h"
#include "shader.h"

struct ImDrawData;
typedef void* ImTextureID; 

namespace Rendering {

struct ImGuiRenderer : private Core::Pinned<ImGuiRenderer> {
    Gfx::Device* device_ = nullptr;
    Core::Box<Gfx::IPipelineBuilder> pipeline_;

    Gfx::Resource font_texture_;
    Gfx::DescriptorHandle font_texture_srv_;

    struct FrameData {
        Gfx::Resource vertex_buffer_;
        Gfx::Resource index_buffer_;
        Containers::Array<Gfx::Resource> constant_buffers_;
        Gfx::Waitable waitable_;
    };
    Containers::Array<FrameData> frame_data_queue_;
    
    void Init(Gfx::Device* device);
    void Shutdown();

    Containers::Hashmap<ImTextureID, Gfx::DescriptorHandle> handles_;
    ImTextureID RegisterHandle(Gfx::DescriptorHandle srv);

    void _CreateFontsTexture();
    void _CreatePipeline();
    void _SetupRenderState(ImDrawData* draw_data, Gfx::Encoder* encoder, FrameData* frame_data);

    void RenderDrawData(ImDrawData* draw_data, Gfx::Encoder* encoder, D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle);
};

}