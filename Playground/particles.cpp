#include "pch.h"

#include "particles.h"
#include "shader.h"
#include "rendering.h"

using namespace Containers;

namespace Rendering {

	Vector3 RandomPointInTriangle(f32 r1, f32 r2, Vector3 a, Vector3 b, Vector3 c) {
		f32 sqrt_r1 = sqrtf(r1);
		return (1.f -sqrt_r1) * a + sqrt_r1 * (1 - r2) * b + r2 * sqrt_r1 * c;
	}

	i32 PolygonParticleGenerator::NumParticles() const {
		return num_particles_;
	}

	void PolygonParticleGenerator::_CreatePipelines() {
		Gfx::ShaderBlob* compiled_shader = Gfx::CompileShaderFromFile(
			L"../data/particle.hlsl",
			L"../data/particle.hlsl",
			L"ParticleDepthPassCs",
			L"cs_6_1"
		);
		D3D12_SHADER_BYTECODE CS;
		CS.pShaderBytecode = compiled_shader->GetBufferPointer();
		CS.BytecodeLength = compiled_shader->GetBufferSize();
		depth_pass_pipeline_ = device_->CreateComputePipeline(CS);
		compiled_shader->Release();
	}

	void PolygonParticleGenerator::Init(Gfx::Device * device, i32 max_particles, f32 spawn_rate, f32 max_lifetime) {
		device_ = device;

		assert(max_particles > static_cast<i32>(spawn_rate * max_lifetime));

		i32 pages_num = (max_particles + PAGE_SIZE - 1) / PAGE_SIZE;
		pages_pool_.Resize(pages_num);

		inv_spawn_rate_ = 1.f / spawn_rate;
		max_lifetime_ = max_lifetime;

		state_positions_texture_ = device->CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, {PAGE_SIZE, pages_num}, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);

		_CreatePipelines();
	}

	void PolygonParticleGenerator::AllocateScreenTextures(Vector2i resolution) {
		if(resolution != resolution_) {
			resolution_ = resolution;

			depth_target_ = device_->CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, resolution, DXGI_FORMAT_R32_UINT, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}
	}

	void PolygonParticleGenerator::_UpdateLifetimes(f32 time_delta) {
		Array<i32> retire_pages;

		for(i32 page_index : active_pages_) {
			for(i32 p = 0; p<PAGE_SIZE; p++) { // could use forward scan
				static_assert(sizeof(LONG) == sizeof(i32)); 
				// TODO: why can't I use i32 as long?
				_bittest(reinterpret_cast<LONG*>(&(pages_pool_[page_index].active_mask)), p);
				if(pages_pool_[page_index].active_mask & (1 << p)) {
					pages_pool_[page_index].lifetimes[p] += time_delta;

					if(pages_pool_[page_index].lifetimes[p] >= max_lifetime_) {
						_bittestandreset(reinterpret_cast<LONG*>(&pages_pool_[page_index].active_mask), p);
						num_particles_--;
					}
				}
			}

			if(pages_pool_[page_index].active_mask == 0) {
				retire_pages.PushBack(page_index);
			}
		}

		// TODO: not the fastest
		for(i32 page_index : retire_pages) {
			for(i32 i = 0; i< active_pages_.Size(); i++) {
				if(active_pages_[i] == page_index) {
					active_pages_.RemoveAndSwapWithLast(i);
					break;
				}
			}
		}
	}

	void PolygonParticleGenerator::_InitParticle(i32 page_index, i32 index) {
		_bittestandset(reinterpret_cast<LONG*>(&pages_pool_[page_index].active_mask), index);
		pages_pool_[page_index].lifetimes[index] = 0.f;
		num_particles_++;

		updates_.PushBack({.page_index = page_index, .index = index, .position = _GeneratePoint() });
	}

	void PolygonParticleGenerator::_SpawnParticle() {
		for(i32 page_index : active_pages_) {
			if(pages_pool_[page_index].active_mask != -1) {
				u32 index;
				static_assert(sizeof(ULONG) == sizeof(u32)); 
				verify(_BitScanForward(reinterpret_cast<ULONG*>(&index), ~pages_pool_[page_index].active_mask));
				_InitParticle(page_index, index);
				return;
			}
		}

		// TODO: again, could be done faster
		for(i32 page_index = 0; page_index < pages_pool_.Size(); page_index++) {
			if(pages_pool_[page_index].active_mask == 0) {
				_InitParticle(page_index, 0);
				active_pages_.PushBack(page_index);
				return;
			}
		}
	}

	void PolygonParticleGenerator::Tick(f32 time_delta) {
		_UpdateLifetimes(time_delta);

		accumulated_time_ += time_delta;

		while(accumulated_time_ >= inv_spawn_rate_) {
			_SpawnParticle();
			accumulated_time_ -= inv_spawn_rate_;
		}

		// UpdateTextures
	}

	void PolygonParticleGenerator::AddPassesToGraph(Gfx::Resource * color_target) {
		if(updates_.Size()) {
			update_positions_pass_ = device_->graph_.AddSubsequentPass(Gfx::PassAttachments{}
				.Attach({ .resource = *state_positions_texture_.resource_ }, D3D12_RESOURCE_STATE_COPY_DEST)
			);
		}

		if(active_pages_.Size()) {
			particle_depth_pass_ = device_->graph_.AddSubsequentPass(Gfx::PassAttachments{}
				.Attach({ .resource = *state_positions_texture_.resource_ }, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
				.Attach({ .resource = *depth_target_.resource_ }, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
				.Attach({ .resource = *color_target->resource_ }, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
			);
		}
	}

	void PolygonParticleGenerator::Render(Gfx::Encoder * encoder, Rendering::Viewport * viewport, Gfx::Resource * color_texture) {
		while(frame_data_queue_.Size() && frame_data_queue_.First().waitable_.IsDone()) {
			frame_data_queue_.RemoveAt(0);
		}

		FrameData frame_data;

		if(updates_.Size()) {
			encoder->SetPass(update_positions_pass_);
			update_positions_pass_ = nullptr;

		
			frame_data.upload_buffer_ = device_->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, updates_.Size() * 256, DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ);
			u8 * positions = nullptr;
			verify_hr(frame_data.upload_buffer_.resource_->Map(0, nullptr, reinterpret_cast<void**>(&positions)));
			D3D12_TEXTURE_COPY_LOCATION copy_src {
				.pResource = *frame_data.upload_buffer_.resource_,
				.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
				.PlacedFootprint = {
					.Footprint = {
						.Format = DXGI_FORMAT_R32G32B32_FLOAT,
						.Width = 1,
						.Height = static_cast<u32>(updates_.Size()),
						.Depth = 1,
						.RowPitch = 256,
					}
				}
			};

			D3D12_TEXTURE_COPY_LOCATION copy_dst {
				.pResource = *state_positions_texture_.resource_,
				.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX
			};
			i32 index = 0;
			for(ParticleInitData update : updates_) {
				memcpy(positions, &update.position, sizeof(update.position));
				positions+=256;

				D3D12_BOX src_box { .left = 0, .top = static_cast<u32>(index), .right = 1, .bottom = static_cast<u32>(index + 1), .back = 1};
				encoder->GetCmdList()->CopyTextureRegion(&copy_dst, update.index, update.page_index, 0, &copy_src, &src_box);
				index++;
			}
			frame_data.upload_buffer_.resource_->Unmap(0, nullptr);
		}
		updates_.Clear();

		if(active_pages_.Size()) {
			encoder->SetPass(particle_depth_pass_);
			particle_depth_pass_ = nullptr;

			frame_data.page_buffer_ = device_->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, active_pages_.Size() * sizeof(i32), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ);

			i32* page_dst = nullptr;
			verify_hr(frame_data.page_buffer_.resource_->Map(0, nullptr, reinterpret_cast<void**>(&page_dst)));
			memcpy(page_dst, active_pages_.Data(), active_pages_.Size() * sizeof(i32));
			frame_data.page_buffer_.resource_->Unmap(0, nullptr);

			{
				D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{
					.Format = DXGI_FORMAT_UNKNOWN,
					.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
					.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
					.Buffer = {
						.NumElements = static_cast<u32>(active_pages_.Size()),
						.StructureByteStride = 2,
					}
				};
				device_->device_->CreateShaderResourceView(*frame_data.page_buffer_.resource_, &srv_desc, encoder->ReserveComputeSlot(Gfx::DescriptorType::SRV, 0));
			}
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{
					.Format = DXGI_FORMAT_R32G32B32_FLOAT,
					.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
					.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
					.Texture2D = {
						.MipLevels = 1
					}
				};
				device_->device_->CreateShaderResourceView(*state_positions_texture_.resource_, &srv_desc, encoder->ReserveComputeSlot(Gfx::DescriptorType::SRV, 1));
			}

			{
				//depth
				D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{
					.Format = DXGI_FORMAT_R32_UINT,
					.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D,
					.Texture2D = {}
				};
				device_->device_->CreateUnorderedAccessView(*depth_target_.resource_, nullptr, &uav_desc, encoder->ReserveComputeSlot(Gfx::DescriptorType::UAV, 0));
			}
			{
				//color
				D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{
					.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
					.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D,
					.Texture2D = {}
				};
				device_->device_->CreateUnorderedAccessView(*color_texture->resource_, nullptr, &uav_desc, encoder->ReserveComputeSlot(Gfx::DescriptorType::UAV, 1));
			}
		
			// TODO: less boilerplate code for data upload
			struct FrameConstants {
				Vector2i resolution;
				int padding[2];
				Matrix4x4 view_projection_matrix;
			};
			static_assert(offsetof(FrameConstants, view_projection_matrix) == 16);
			FrameConstants frame_constants {
				.resolution = viewport->resolution,
				.view_projection_matrix = viewport->view_projection_matrix
			};

			frame_data.constant_buffers_.PushBackRvalueRef(std::move(device_->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, AlignedForward(static_cast<i32>(sizeof(frame_constants)), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ)));

			void* cb_dst = nullptr;
			verify_hr(frame_data.constant_buffers_.Last().resource_->Map(0, nullptr, &cb_dst));
			memcpy(cb_dst, &frame_constants, sizeof(frame_constants));
			frame_data.constant_buffers_.Last().resource_->Unmap(0, nullptr);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc{
				.BufferLocation = frame_data.constant_buffers_.Last().resource_->GetGPUVirtualAddress(),
				.SizeInBytes = AlignedForward(static_cast<u32>(sizeof(frame_constants)), static_cast<u32>(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT))
			};
			device_->device_->CreateConstantBufferView(&cbv_desc, encoder->ReserveComputeSlot(Gfx::DescriptorType::CBV, 0));
		
			encoder->GetCmdList()->SetPipelineState(*depth_pass_pipeline_.pipeline_);
			encoder->SetComputeDescriptors();
			encoder->GetCmdList()->Dispatch(static_cast<u32>(active_pages_.Size() * PAGE_SIZE), 1, 1);
		}

		frame_data.waitable_ = encoder->GetWaitable();

		frame_data_queue_.PushBackRvalueRef(std::move(frame_data));
	}

	Vector3 PolygonParticleGenerator::_GeneratePoint() {
		Array<f32> triangle_sizes;

		assert(vertices_.Size() % 3 == 0);

		for(i32 t = 0; t < vertices_.Size(); t += 3) {
			triangle_sizes.PushBack(0.5f * fabsf(Math::cross(vertices_[t + 1] - vertices_[t + 0], vertices_[t + 2] - vertices_[t + 0]).length()));
		}

		for(i32 i = 1; i < triangle_sizes.Size(); i++) {
			triangle_sizes[i] += triangle_sizes[i-1];
		}

		f32 r = rng_.F32UniformInRange(0.f, triangle_sizes.Last());
		i64 index = Algorithms::LowerBound(triangle_sizes.Data(), triangle_sizes.Size(), r);

		return RandomPointInTriangle(rng_.F32Uniform(), rng_.F32Uniform(), vertices_[3 * index + 0], vertices_[3 * index + 1], vertices_[3 * index + 2]);
	}
}
