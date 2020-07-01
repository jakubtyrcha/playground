#pragma once

#include "gfx.h"
#include "random.h"

namespace Rendering {
	struct Viewport;

	struct PolygonParticleGenerator {
		Gfx::Device* device_ = nullptr;

		Containers::Array<Vector3> vertices_; // tri list
		Random::Generator rng_;
		static constexpr i32 PAGE_SIZE = 32;
		struct ParticlePage {
			i32 active_mask;
			f32 lifetimes[PAGE_SIZE];
		};
		f32 inv_spawn_rate_; // in seconds/particle
		f32 max_lifetime_ = 0; // in seconds
		f32 accumulated_time_ = 0.f;
		Containers::Array<ParticlePage> pages_pool_;
		Containers::Array<i32> active_pages_;
		i32 num_particles_ = 0;

		Gfx::Resource position_texture_;

		Vector2i resolution_;
		Gfx::Resource depth_target_;
		Gfx::Resource state_positions_texture_;

		Gfx::Pipeline depth_pass_pipeline_;

		Gfx::Pass* update_positions_pass_ = nullptr;
		Gfx::Pass* particle_depth_pass_ = nullptr;

		struct FrameData {
			Gfx::Resource page_buffer_;
			Gfx::Resource upload_buffer_;
			Containers::Array<Gfx::Resource> constant_buffers_;
			Gfx::Waitable waitable_;
		};

		i32 max_frames_queued_ = 3;
		Containers::Array<FrameData> frame_data_queue_;

		struct ParticleInitData {
			i32 page_index;
			i32 index;
			Vector3 position;
		};
		Containers::Array<ParticleInitData> updates_;

		void _CreatePipelines();
		Vector3 _GeneratePoint();
		void _InitParticle(i32 page_index, i32 index);
		void _SpawnParticle();
		void _UpdateLifetimes(f32 time_delta);

		i32 NumParticles() const;
		void Init(Gfx::Device * device, i32 max_particles, f32 spawn_rate, f32 max_lifetime);
		void AllocateScreenTextures(Vector2i resolution);
		void Tick(f32 time_delta);
		void AddPassesToGraph(Gfx::Resource * color_target);
		void Render(Gfx::Encoder * encoder, Viewport * viewport, Gfx::Resource * color_texture);
	};
}
