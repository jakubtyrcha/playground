#pragma once

#include "gfx.h"
#include "array.h"
#include "shader.h"

namespace Rendering {
	struct Viewport;

	using ColourR8G8B8A8U = Magnum::Math::Vector4<u8>;

	struct ShapeVertex {
		Vector3 position;
		ColourR8G8B8A8U colour;
	};

	struct ImmediateModeShapeRenderer {
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
		Containers::Array<ShapeVertex> vertices_;

		void Init(Gfx::Device* device);
		void Render(Gfx::Encoder* encoder, Viewport* viewport, D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle);
		void Shutdown();
		void AddLine(Vector3 a, Vector3 b, ColourR8G8B8A8U colour);
	};
}