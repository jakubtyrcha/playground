#pragma once

#include "gfx.h"
#include "array.h"

namespace Rendering {
	struct Viewport;

	struct ImmediateModeShapeRenderer {
		Gfx::Device* device_ = nullptr;
		Gfx::Pipeline pipeline_;

		struct FrameData {
			Gfx::Resource vertex_buffer_;
			Gfx::Resource index_buffer_;
			Containers::Array<Gfx::Resource> constant_buffers_;
			Gfx::Waitable waitable_;
		};

		i32 max_frames_queued_ = 3;
		Containers::Array<FrameData> frame_data_queue_;

		void Init(Gfx::Device* device);
		void Render(Gfx::Encoder* encoder, Viewport* viewport, D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle);
		void Shutdown();
	};
}