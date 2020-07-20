#include "pch.h"

#include "particles.h"
#include "rendering.h"
#include "shader.h"

using namespace Core;
using namespace Containers;

namespace Rendering {

Vector3 RandomPointInTriangle(f32 r1, f32 r2, Vector3 a, Vector3 b, Vector3 c)
{
    f32 sqrt_r1 = sqrtf(r1);
    return (1.f - sqrt_r1) * a + sqrt_r1 * (1 - r2) * b + r2 * sqrt_r1 * c;
}

void PolygonParticleGenerator::Spawn(i32 num)
{
    for (i32 i = 0; i < num; i++) {
        _SpawnParticle();
    }
}

i32 PolygonParticleGenerator::NumParticles() const
{
    return num_particles_;
}

struct ParticlePipeline : public Gfx::IPipelineBuilder {
    PolygonParticleGenerator* owner_ = nullptr;

    ParticlePipeline(PolygonParticleGenerator* owner)
        : owner_(owner)
    {
    }

    Optional<Box<Gfx::Pipeline>> Build() override
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc {};
        pso_desc.NodeMask = 1;
        pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pso_desc.pRootSignature = *owner_->device_->root_signature_;
        pso_desc.SampleMask = UINT_MAX;
        pso_desc.NumRenderTargets = 2;
        pso_desc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
        pso_desc.RTVFormats[1] = DXGI_FORMAT_R16G16_FLOAT;
        pso_desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        pso_desc.SampleDesc.Count = 1;
        pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

        pso_desc.VS = GetShaderFromShaderFileSource({ .file_path = L"../data/sphere_splatting.hlsl",
                                                        .entrypoint = L"VsMain",
                                                        .profile = L"vs_6_0" })
                          ->GetBytecode();

        pso_desc.PS = GetShaderFromShaderFileSource({ .file_path = L"../data/sphere_splatting.hlsl",
                                                        .entrypoint = L"PsMain",
                                                        .profile = L"ps_6_0" })
                          ->GetBytecode();

        // Create the blending setup
        {
            D3D12_BLEND_DESC& desc = pso_desc.BlendState;
            desc.AlphaToCoverageEnable = false;
            desc.RenderTarget[0].BlendEnable = false;
            desc.RenderTarget[0].LogicOpEnable = false;
            desc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        }

        // Create the rasterizer state
        {
            D3D12_RASTERIZER_DESC& desc = pso_desc.RasterizerState;
            desc.FillMode = D3D12_FILL_MODE_SOLID;
            desc.CullMode = D3D12_CULL_MODE_NONE;
            desc.FrontCounterClockwise = FALSE;
            desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
            desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            desc.DepthClipEnable = true;
            desc.MultisampleEnable = FALSE;
            desc.AntialiasedLineEnable = FALSE;
            desc.ForcedSampleCount = 0;
            desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        }

        // Create depth-stencil State
        {
            D3D12_DEPTH_STENCIL_DESC& desc = pso_desc.DepthStencilState;
            desc.DepthEnable = true;
            desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
            desc.StencilEnable = false;
            desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
            desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            desc.BackFace = desc.FrontFace;
        }

        return Gfx::Pipeline::From(owner_->device_, pso_desc);
    }
};

void PolygonParticleGenerator::Init(Gfx::Device* device, i32 max_particles, f32 spawn_rate, f32 max_lifetime)
{
    device_ = device;

    assert(max_particles > static_cast<i32>(spawn_rate * max_lifetime));

    i32 pages_num = (max_particles + PAGE_SIZE - 1) / PAGE_SIZE;
    pages_pool_.Resize(pages_num);

    inv_spawn_rate_ = 1.f / spawn_rate;
    max_lifetime_ = max_lifetime;

    state_positions_texture_ = device->CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, { PAGE_SIZE, pages_num }, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);

    pipeline_ = MakeBox<ParticlePipeline>(this);
}

void PolygonParticleGenerator::_UpdateLifetimes(f32 time_delta)
{
    Array<i32> retire_pages;

    for (i32 page_index : active_pages_) {
        for (i32 p = 0; p < PAGE_SIZE; p++) { // could use forward scan
            static_assert(sizeof(LONG) == sizeof(i32));
            // TODO: why can't I use i32 as long?
            _bittest(reinterpret_cast<LONG*>(&(pages_pool_[page_index].active_mask)), p);
            if (pages_pool_[page_index].active_mask & (1 << p)) {
                pages_pool_[page_index].lifetimes[p] += time_delta;

                if (pages_pool_[page_index].lifetimes[p] >= max_lifetime_) {
                    _bittestandreset(reinterpret_cast<LONG*>(&pages_pool_[page_index].active_mask), p);
                    num_particles_--;
                }
            }
        }

        if (pages_pool_[page_index].active_mask == 0) {
            retire_pages.PushBack(page_index);
        }
    }

    // TODO: not the fastest
    for (i32 page_index : retire_pages) {
        for (i32 i = 0; i < active_pages_.Size(); i++) {
            if (active_pages_[i] == page_index) {
                active_pages_.RemoveAtAndSwapWithLast(i);
                break;
            }
        }
    }
}

void PolygonParticleGenerator::_InitParticle(i32 page_index, i32 index)
{
    _bittestandset(reinterpret_cast<LONG*>(&pages_pool_[page_index].active_mask), index);
    pages_pool_[page_index].lifetimes[index] = 0.f;
    num_particles_++;

    updates_.PushBack({ .page_index = page_index, .index = index, .position = _GeneratePoint() });
}

void PolygonParticleGenerator::_SpawnParticle()
{
    for (i32 page_index : active_pages_) {
        if (pages_pool_[page_index].active_mask != -1) {
            u32 index;
            static_assert(sizeof(ULONG) == sizeof(u32));
            verify(_BitScanForward(reinterpret_cast<ULONG*>(&index), ~pages_pool_[page_index].active_mask));
            _InitParticle(page_index, index);
            return;
        }
    }

    // TODO: again, could be done faster
    for (i32 page_index = 0; page_index < pages_pool_.Size(); page_index++) {
        if (pages_pool_[page_index].active_mask == 0) {
            _InitParticle(page_index, 0);
            active_pages_.PushBack(page_index);
            return;
        }
    }
}

void PolygonParticleGenerator::Tick(f32 time_delta)
{
    _UpdateLifetimes(time_delta);

    accumulated_time_ += time_delta;

    while (accumulated_time_ >= inv_spawn_rate_ && num_particles_ < pages_pool_.Size() * PAGE_SIZE) {
        _SpawnParticle();
        accumulated_time_ -= inv_spawn_rate_;
    }

    // UpdateTextures
}

void PolygonParticleGenerator::AddPassesToGraph(Gfx::Resource* color_target, Gfx::Resource* depth_target, Gfx::Resource* motion_vectors_target)
{
    if (updates_.Size()) {
        update_positions_pass_ = device_->graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                                       .Attach({ .resource = *state_positions_texture_.resource_ }, D3D12_RESOURCE_STATE_COPY_DEST));
    }

    if (active_pages_.Size()) {
        particle_pass_ = device_->graph_.AddSubsequentPass(Gfx::PassAttachments {}
                                                               .Attach({ .resource = *state_positions_texture_.resource_ }, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
                                                               .Attach({ .resource = *depth_target->resource_ }, D3D12_RESOURCE_STATE_DEPTH_WRITE)
                                                               .Attach({ .resource = *color_target->resource_ }, D3D12_RESOURCE_STATE_RENDER_TARGET)
                                                               .Attach({ .resource = *motion_vectors_target->resource_ }, D3D12_RESOURCE_STATE_RENDER_TARGET));
    }
}

void PolygonParticleGenerator::Render(Gfx::Encoder* encoder, Rendering::Viewport* viewport, D3D12_CPU_DESCRIPTOR_HANDLE color_target_handle, D3D12_CPU_DESCRIPTOR_HANDLE depth_target_handle, D3D12_CPU_DESCRIPTOR_HANDLE motion_vectors_target_handle)
{
    while (frame_data_queue_.Size() && frame_data_queue_.First().waitable_.IsDone()) {
        frame_data_queue_.RemoveAt(0);
    }

    FrameData frame_data;

    if (updates_.Size()) {
        encoder->SetPass(update_positions_pass_);
        update_positions_pass_ = nullptr;

        frame_data.upload_buffer_ = device_->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, updates_.Size() * 256, DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ);
        u8* positions = nullptr;
        verify_hr(frame_data.upload_buffer_.resource_->Map(0, nullptr, reinterpret_cast<void**>(&positions)));
        D3D12_TEXTURE_COPY_LOCATION copy_src {
            .pResource = *frame_data.upload_buffer_.resource_,
            .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
            .PlacedFootprint = {
                .Footprint = {
                    .Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
                    .Width = 1,
                    .Height = static_cast<u32>(updates_.Size()),
                    .Depth = 1,
                    .RowPitch = 256,
                } }
        };

        const float PARTICLE_RADIUS = 0.25f;

        D3D12_TEXTURE_COPY_LOCATION copy_dst {
            .pResource = *state_positions_texture_.resource_,
            .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX
        };
        i32 index = 0;
        for (ParticleInitData update : updates_) {
            //memcpy(positions, &update.position, sizeof(update.position));
            *reinterpret_cast<Vector4*>(positions) = Vector4 { update.position, PARTICLE_RADIUS };
            positions += 256;

            D3D12_BOX src_box { .left = 0, .top = static_cast<u32>(index), .right = 1, .bottom = static_cast<u32>(index + 1), .back = 1 };
            encoder->GetCmdList()->CopyTextureRegion(&copy_dst, update.index, update.page_index, 0, &copy_src, &src_box);
            index++;
        }
        frame_data.upload_buffer_.resource_->Unmap(0, nullptr);
    }
    updates_.Clear();

    if (active_pages_.Size()) {
        encoder->SetPass(particle_pass_);
        particle_pass_ = nullptr;

        frame_data.page_buffer_ = device_->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, active_pages_.Size() * sizeof(i32), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ);

        i32* page_dst = nullptr;
        verify_hr(frame_data.page_buffer_.resource_->Map(0, nullptr, reinterpret_cast<void**>(&page_dst)));
        memcpy(page_dst, active_pages_.Data(), active_pages_.Size() * sizeof(i32));
        frame_data.page_buffer_.resource_->Unmap(0, nullptr);

        {
            D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc {
                .Format = DXGI_FORMAT_UNKNOWN,
                .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
                .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
                .Buffer = {
                    .NumElements = static_cast<u32>(active_pages_.Size()),
                    .StructureByteStride = 2,
                }
            };
            device_->device_->CreateShaderResourceView(*frame_data.page_buffer_.resource_, &srv_desc, encoder->ReserveGraphicsSlot(Gfx::DescriptorType::SRV, 0));
        }
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc {
                .Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
                .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
                .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
                .Texture2D = {
                    .MipLevels = 1 }
            };
            device_->device_->CreateShaderResourceView(*state_positions_texture_.resource_, &srv_desc, encoder->ReserveGraphicsSlot(Gfx::DescriptorType::SRV, 1));
        }

        // TODO: less boilerplate code for data upload
        struct FrameConstants {
            Vector2i resolution;
            Vector2 inv_resolution;
            Vector2 near_far_planes;
            Vector2 clipspace_jitter;
            Matrix4 view_matrix;
            Matrix4 projection_matrix;
            Matrix4 view_projection_matrix;
            Matrix4 inv_view_projection_matrix;
            Matrix4 inv_view_matrix;
            Matrix4 prev_view_projection_matrix;
        };
        FrameConstants constants;
        constants.resolution = viewport->resolution;
        constants.inv_resolution = 1.f / Vector2 { constants.resolution };
        constants.near_far_planes = Vector2 { viewport->near_plane, viewport->far_plane };
        if (viewport->taa_offsets.Size()) {
            constants.clipspace_jitter = viewport->projection_jitter;
        } else {
            constants.clipspace_jitter = {};
        }
        constants.view_matrix = viewport->view_matrix;
        constants.projection_matrix = viewport->projection_matrix;
        constants.view_projection_matrix = viewport->view_projection_matrix;
        constants.inv_view_projection_matrix = viewport->inv_view_projection_matrix;
        constants.inv_view_matrix = viewport->inv_view_matrix;
        constants.prev_view_projection_matrix = viewport->prev_view_projection_matrix;

        frame_data.constant_buffers_.PushBackRvalueRef(std::move(device_->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, AlignedForward(static_cast<i32>(sizeof(FrameConstants)), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ)));
        void* cb_dst = nullptr;
        verify_hr(frame_data.constant_buffers_.Last().resource_->Map(0, nullptr, &cb_dst));
        memcpy(cb_dst, &constants, sizeof(FrameConstants));
        frame_data.constant_buffers_.Last().resource_->Unmap(0, nullptr);

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc {
            .BufferLocation = frame_data.constant_buffers_.Last().resource_->GetGPUVirtualAddress(),
            .SizeInBytes = static_cast<u32>(AlignedForward(static_cast<i32>(sizeof(FrameConstants)), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT))
        };
        device_->device_->CreateConstantBufferView(&cbv_desc, encoder->ReserveGraphicsSlot(Gfx::DescriptorType::CBV, 0));

        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handles[] = {color_target_handle, motion_vectors_target_handle};
        encoder->GetCmdList()->OMSetRenderTargets(_countof(rtv_handles), rtv_handles, false, &depth_target_handle);

        D3D12_VIEWPORT vp {
            .Width = static_cast<float>(viewport->resolution.x()),
            .Height = static_cast<float>(viewport->resolution.y()),
            .MinDepth = 0.f,
            .MaxDepth = 1.f
        };
        encoder->GetCmdList()->RSSetViewports(1, &vp);

        encoder->GetCmdList()->IASetVertexBuffers(0, 1, nullptr);
        encoder->GetCmdList()->IASetIndexBuffer(nullptr);
        encoder->GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        encoder->GetCmdList()->SetPipelineState(pipeline_->GetPSO());
        encoder->SetGraphicsDescriptors();
        //encoder->GetCmdList()->Dispatch(static_cast<u32>(active_pages_.Size() * PAGE_SIZE), 1, 1);
        const D3D12_RECT r = { 0, 0, viewport->resolution.x(), viewport->resolution.y() };
        encoder->GetCmdList()->RSSetScissorRects(1, &r);
        encoder->GetCmdList()->DrawInstanced(4, static_cast<u32>(active_pages_.Size() * PAGE_SIZE), 0, 0);
    }

    frame_data.waitable_ = encoder->GetWaitable();

    frame_data_queue_.PushBackRvalueRef(std::move(frame_data));
}

Vector3 PolygonParticleGenerator::_GeneratePoint()
{
    Array<f32> triangle_sizes;

    assert(vertices_.Size() % 3 == 0);

    for (i32 t = 0; t < vertices_.Size(); t += 3) {
        triangle_sizes.PushBack(0.5f * fabsf(Math::cross(vertices_[t + 1] - vertices_[t + 0], vertices_[t + 2] - vertices_[t + 0]).length()));
    }

    for (i32 i = 1; i < triangle_sizes.Size(); i++) {
        triangle_sizes[i] += triangle_sizes[i - 1];
    }

    f32 r = rng_.F32UniformInRange(0.f, triangle_sizes.Last());
    i64 index = Algorithms::LowerBound(triangle_sizes.Data(), triangle_sizes.Size(), r);

    return RandomPointInTriangle(rng_.F32Uniform(), rng_.F32Uniform(), vertices_[3 * index + 0], vertices_[3 * index + 1], vertices_[3 * index + 2]);
}
}
