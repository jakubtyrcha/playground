#pragma once

#include "gfx.h"
#include "random.h"
#include "shader.h"
#include "box.h"

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
    Gfx::Resource state_positions_texture_;

    Core::Box<Gfx::IPipelineBuilder> pipeline_;

    Gfx::Pass* update_positions_pass_ = nullptr;
    Gfx::Pass* particle_pass_ = nullptr;

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

    Vector3 _GeneratePoint();
    void _InitParticle(i32 page_index, i32 index);
    void _SpawnParticle();
    void _UpdateLifetimes(f32 time_delta);

    void Spawn(i32 num);
    i32 NumParticles() const;
    void Init(Gfx::Device* device, i32 max_particles, f32 spawn_rate, f32 max_lifetime);
    void Tick(f32 time_delta);
    void AddPassesToGraph(Gfx::Resource* color_target, Gfx::Resource* depth_target);
    void Render(Gfx::Encoder* encoder, Viewport* viewport, D3D12_CPU_DESCRIPTOR_HANDLE color_target_handle, D3D12_CPU_DESCRIPTOR_HANDLE depth_target_handle);
};
}
