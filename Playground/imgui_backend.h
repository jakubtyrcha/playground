#pragma once

#include "gfx.h"
#include "shader.h"

struct ImDrawData;

namespace Rendering {

	struct ImGuiRenderer : private Pinned<ImGuiRenderer> {
		Gfx::Resource font_texture_;
		Gfx::Device* device_ = nullptr;
		Box<Gfx::IPipelineBuilder> pipeline_;

		struct FrameData {
			Gfx::Resource vertex_buffer_;
			Gfx::Resource index_buffer_;
			Containers::Array<Gfx::Resource> constant_buffers_;
			Gfx::Waitable waitable_;
		};

		i32 max_frames_queued_ = 3;
		Containers::Array<FrameData> frame_data_queue_;

		struct ViewportData {
		};

		ViewportData main_viewport_;

		void Init(Gfx::Device* device);
		void Shutdown();

		void _CreateFontsTexture();
		void _CreatePipeline();
		void _SetupRenderState(ImDrawData* draw_data, Gfx::Encoder* encoder, FrameData* frame_data);

		void RenderDrawData(ImDrawData* draw_data, Gfx::Encoder* encoder, D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle);
	};

}