#include "pch.h"

#include "gfx.h"
#include <fmt/format.h>
#include "array.h"
#include "assertions.h"
#include "os.h"
#include "files.h"

#include "shader.h"
#include <dxcapi.h>

#include <imgui/imgui.h>
#include "imgui_impl_win32.h"

#include <math.h>

using namespace Containers;
using namespace IO;

using namespace Magnum;
namespace Rendering {
	struct Viewport {
		Vector2i resolution;
		f32 fov_y;
		Vector3 camera_look_at;
		Vector3 camera_position;
		Vector3 camera_up;
		f32 near_plane;
		f32 far_plane;

		f32 GetAspectRatio() const;
		Vector3 GetLookToVector() const;
		Vector3 GetRightVector() const;
		void TranslateCamera(Vector3);

		Matrix4x4 view_matrix;
		Matrix4x4 inv_view_matrix;
		Matrix4x4 camera_offseted_view_matrix;
		Matrix4x4 camera_offseted_inv_view_matrix;
		Matrix4x4 projection_matrix;
		Matrix4x4 inv_projection_matrix;
		Matrix4x4 view_projection_matrix;
		Matrix4x4 inv_view_projection_matrix;
		Matrix4x4 camera_offseted_view_projection_matrix;
		Matrix4x4 inv_camera_offseted_view_projection_matrix;
		Matrix4x4 prev_view_matrix;
		Matrix4x4 prev_inv_view_matrix;
	};

	f32 Viewport::GetAspectRatio() const {
		return resolution.x() / static_cast<f32>(resolution.y());
	}

	Vector3 Viewport::GetLookToVector() const {
		return (camera_look_at - camera_position).normalized();
	}

	Vector3 Viewport::GetRightVector() const {
		Vector3 z = (camera_look_at - camera_position).normalized();
		return Math::cross(camera_up, z).normalized();
	}

	void Viewport::TranslateCamera(Vector3 v) {
		camera_position += v;
		camera_look_at += v;
	}

	Matrix4 LookAtLh(Vector3 look_at, Vector3 eye, Vector3 up) {
		Vector3 z = (look_at - eye).normalized();
		Vector3 x = Math::cross(up, z).normalized();
		Vector3 y = Math::cross(z, x);

		return Matrix4{ Vector4{x, -Math::dot(x, eye)}, Vector4{y, -Math::dot(y, eye)},Vector4{z, -Math::dot(z, eye)},Vector4{0, 0, 0, 1} }.transposed();
	}

	Matrix4 InverseLookAtLh(Vector3 look_at, Vector3 eye, Vector3 up) {
		Matrix4 view = LookAtLh(look_at, eye, up);
		return Matrix4::from(view.rotation().transposed(), eye);
	}

	Matrix4 PerspectiveFovLh(f32 aspect_ratio, f32 fov_y, f32 near_plane, f32 far_plane) {
		f32 yscale = 1.f / tanf(fov_y * 0.5f);
		f32 xscale = yscale / aspect_ratio;

		return {
			Vector4{xscale, 0, 0, 0},
			Vector4{0, yscale, 0, 0},
			Vector4{0, 0, far_plane / (far_plane - near_plane), 1},
			Vector4{0, 0, -near_plane * far_plane / (far_plane - near_plane), 0}
		};
	}

	Matrix4 InversePerspectiveFovLh(f32 aspect_ratio, f32 fov_y, f32 near_plane, f32 far_plane) {
		f32 yscale = tanf(fov_y * 0.5f);
		f32 xscale = yscale * aspect_ratio;
		f32 ab = -1.f / near_plane;

		return {
			Vector4{xscale, 0, 0, 0},
			Vector4{0, yscale, 0, 0},
			Vector4{0, 0, 0, ab},
			Vector4{0, 0, 1, -ab}
		};
	}

	Matrix4 PerspectiveFovLhReversedZ(f32 aspect_ratio, f32 fov_y, f32 near, f32 far);

	Matrix4 InversePerspectiveFovLhReversedZ(f32 aspect_ratio, f32 fov_y, f32 near, f32 far);

	struct ImmediateModeShapeRenderer {
		Gfx::Device* device_ = nullptr;
		Gfx::Pipeline pipeline_;

		struct FrameData {
			Gfx::Resource vertex_buffer_;
			Gfx::Resource index_buffer_;
			Array<Gfx::Resource> constant_buffers_;
			Gfx::Waitable waitable_;
		};

		i32 max_frames_queued_ = 3;
		Array<FrameData> frame_data_queue_;

		void Init(Gfx::Device* device) {
			device_ = device;

			D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc{};
			pso_desc.NodeMask = 1;
			pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			pso_desc.pRootSignature = *device_->root_signature_;
			pso_desc.SampleMask = UINT_MAX;
			pso_desc.NumRenderTargets = 1;
			pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			pso_desc.SampleDesc.Count = 1;
			pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

			Gfx::ShaderBlob* vs_bytecode;
			{
				vs_bytecode = Gfx::CompileShaderFromFile(L"../data/shape.hlsl", L"../data/shape.hlsl", L"VsMain", L"vs_6_0");
				pso_desc.VS = { vs_bytecode->GetBufferPointer(), vs_bytecode->GetBufferSize() };
			}

			// Create the input layout
			static D3D12_INPUT_ELEMENT_DESC local_layout[] = {
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,   0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			};
			pso_desc.InputLayout = { local_layout, _countof(local_layout) };

			Gfx::ShaderBlob* ps_bytecode;
			{
				ps_bytecode = Gfx::CompileShaderFromFile(L"../data/shape.hlsl", L"../data/shape.hlsl", L"PsMain", L"ps_6_0");
				pso_desc.PS = { ps_bytecode->GetBufferPointer(), ps_bytecode->GetBufferSize() };
			}

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
				desc.AntialiasedLineEnable = TRUE;
				desc.ForcedSampleCount = 0;
				desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
			}

			// Create depth-stencil State
			{
				D3D12_DEPTH_STENCIL_DESC& desc = pso_desc.DepthStencilState;
				desc.DepthEnable = false; //
				desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
				desc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
				desc.StencilEnable = false;
				desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
				desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
				desc.BackFace = desc.FrontFace;
			}

			verify_hr(device_->device_->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(pipeline_.pipeline_.InitAddress())));

			vs_bytecode->Release();
			ps_bytecode->Release();
		}

		void Render(Gfx::Encoder* encoder, Viewport* viewport, D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle) {
			if (frame_data_queue_.Size() > max_frames_queued_) {
				frame_data_queue_.RemoveAt(0);
				// TODO: fancier ahead-of-time waitables
				//frame_data_queue_.RemoveAt(0).waitable_.Wait();
			}

			encoder->GetCmdList()->OMSetRenderTargets(1, &rtv_handle, false, nullptr);

			D3D12_VIEWPORT vp{
				.Width = static_cast<float>(viewport->resolution.x()),
				.Height = static_cast<float>(viewport->resolution.y()),
				.MinDepth = 0.f,
				.MaxDepth = 1.f
			};
			encoder->GetCmdList()->RSSetViewports(1, &vp);

			using ColourR8G8B8A8U = Magnum::Math::Vector4<u8>;
			struct ShapeVertex {
				Vector3 position;
				ColourR8G8B8A8U colour;
			};

			static_assert(offsetof(ShapeVertex, colour) == 12);

			ShapeVertex vertices[] = {
				{.position = {0, 0, 0}, .colour = {255, 0, 0, 1}},
				{.position = {1, 0, 0}, .colour = {255, 0, 0, 1}},
				{.position = {0, 0, 0}, .colour = {0, 255, 0, 1}},
				{.position = {0, 1, 0}, .colour = {0, 255, 0, 1}},
				{.position = {0, 0, 0}, .colour = {0, 0, 255, 1}},
				{.position = {0, 0, 1}, .colour = {0, 0, 255, 1}},
			};

			i32 verticesNum = _countof(vertices);

			FrameData frame_data;
			frame_data.vertex_buffer_ = device_->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, verticesNum * sizeof(ShapeVertex), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ);

			ShapeVertex* vtx_dst = nullptr;
			verify_hr(frame_data.vertex_buffer_.resource_->Map(0, nullptr, reinterpret_cast<void**>(&vtx_dst)));
			memcpy(vtx_dst, vertices, sizeof(vertices));
			frame_data.vertex_buffer_.resource_->Unmap(0, nullptr);

			// Bind shader and vertex buffers
			unsigned int stride = sizeof(ShapeVertex);
			unsigned int offset = 0;
			D3D12_VERTEX_BUFFER_VIEW vbv{
				.BufferLocation = frame_data.vertex_buffer_.resource_->GetGPUVirtualAddress() + offset,
				.SizeInBytes = verticesNum * stride,
				.StrideInBytes = stride
			};
			encoder->GetCmdList()->IASetVertexBuffers(0, 1, &vbv);
			encoder->GetCmdList()->IASetIndexBuffer(nullptr);
			encoder->GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
			encoder->GetCmdList()->SetPipelineState(*pipeline_.pipeline_);

			frame_data.constant_buffers_.PushBackRvalueRef(std::move(device_->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, AlignedForward(64, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ)));

			Matrix4x4 view_projection_matrix = viewport->view_projection_matrix;

			void* cb_dst = nullptr;
			verify_hr(frame_data.constant_buffers_.Last().resource_->Map(0, nullptr, &cb_dst));
			memcpy(cb_dst, &view_projection_matrix, 64);
			frame_data.constant_buffers_.Last().resource_->Unmap(0, nullptr);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc{
				.BufferLocation = frame_data.constant_buffers_.Last().resource_->GetGPUVirtualAddress(),
				.SizeInBytes = static_cast<u32>(AlignedForward(64, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT))
			};
			device_->device_->CreateConstantBufferView(&cbv_desc, encoder->ReserveGraphicsSlot(Gfx::DescriptorType::CBV, 0));

			encoder->SetGraphicsDescriptors();

			const D3D12_RECT r = { 0, 0, viewport->resolution.x(), viewport->resolution.y() };
			encoder->GetCmdList()->RSSetScissorRects(1, &r);
			encoder->GetCmdList()->DrawInstanced(6, 1, 0, 0);

			frame_data_queue_.PushBackRvalueRef(std::move(frame_data));
		}

		void Shutdown() {
			frame_data_queue_.Clear();
		}
	};
};

#include "random.h"

Vector3 RandomPointInTriangle(f32 r1, f32 r2, Vector3 a, Vector3 b, Vector3 c) {
	f32 sqrt_r1 = sqrtf(r1);
	return (1.f -sqrt_r1) * a + sqrt_r1 * (1 - r2) * b + r2 * sqrt_r1 * c;
}

namespace Particle {
	struct PolygonGenerator {
		Array<Vector3> vertices_; // tri list
		Random::Generator rng_;
		static constexpr i32 PAGE_SIZE = 32;
		struct ParticlePage {
			i32 active_mask;
			f32 lifetimes[PAGE_SIZE];
		};
		f32 inv_spawn_rate_; // in seconds/particle
		f32 max_lifetime_ = 0; // in seconds
		f32 accumulated_time_ = 0.f;
		Array<ParticlePage> pages_pool_;
		Array<i32> active_pages_;
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
			Array<Gfx::Resource> constant_buffers_;
			Gfx::Waitable waitable_;
		};

		i32 max_frames_queued_ = 3;
		Array<FrameData> frame_data_queue_;

		struct ParticleInitData {
			i32 page_index;
			i32 index;
			Vector3 position;
		};
		Array<ParticleInitData> updates_;

		i32 NumParticles() const {
			return num_particles_;
		}

		void CreatePipelines(Gfx::Device * device) {
			Gfx::ShaderBlob* compiled_shader = Gfx::CompileShaderFromFile(
				L"../data/particle.hlsl",
				L"../data/particle.hlsl",
				L"ParticleDepthPassCs",
				L"cs_6_1"
			);
			D3D12_SHADER_BYTECODE CS;
			CS.pShaderBytecode = compiled_shader->GetBufferPointer();
			CS.BytecodeLength = compiled_shader->GetBufferSize();
			depth_pass_pipeline_ = device->CreateComputePipeline(CS);
			compiled_shader->Release();
		}

		void Init(Gfx::Device * device, i32 max_particles, f32 spawn_rate, f32 max_lifetime) {
			assert(max_particles > static_cast<i32>(spawn_rate * max_lifetime));

			i32 pages_num = (max_particles + PAGE_SIZE - 1) / PAGE_SIZE;
			pages_pool_.Resize(pages_num);

			inv_spawn_rate_ = 1.f / spawn_rate;
			max_lifetime_ = max_lifetime;

			state_positions_texture_ = device->CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, {PAGE_SIZE, pages_num}, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);
		}

		void AllocateScreenTextures(Gfx::Device* device, Vector2i resolution) {
			if(resolution != resolution_) {
				resolution_ = resolution;

				depth_target_ = device->CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, resolution, DXGI_FORMAT_R32_UINT, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			}
		}

		Vector3 GeneratePoint();

		void Update(f32 time_delta) {
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

		void InitParticle(i32 page_index, i32 index) {
			_bittestandset(reinterpret_cast<LONG*>(&pages_pool_[page_index].active_mask), index);
			pages_pool_[page_index].lifetimes[index] = 0.f;
			num_particles_++;

			updates_.PushBack({.page_index = page_index, .index = index, .position = GeneratePoint() });
		}

		void SpawnParticle() {
			for(i32 page_index : active_pages_) {
				if(pages_pool_[page_index].active_mask != -1) {
					u32 index;
					static_assert(sizeof(ULONG) == sizeof(u32)); 
					verify(_BitScanForward(reinterpret_cast<ULONG*>(&index), ~pages_pool_[page_index].active_mask));
					InitParticle(page_index, index);
					return;
				}
			}

			// TODO: again, could be done faster
			for(i32 page_index = 0; page_index < pages_pool_.Size(); page_index++) {
				if(pages_pool_[page_index].active_mask == 0) {
					InitParticle(page_index, 0);
					active_pages_.PushBack(page_index);
					return;
				}
			}
		}

		void Tick(f32 time_delta) {
			Update(time_delta);

			accumulated_time_ += time_delta;

			while(accumulated_time_ >= inv_spawn_rate_) {
				SpawnParticle();
				accumulated_time_ -= inv_spawn_rate_;
			}

			// UpdateTextures
		}

		void AddPassesToGraph(Gfx::Device * device, Gfx::Resource * color_target) {
			if(updates_.Size()) {
				update_positions_pass_ = device->graph_.AddSubsequentPass(Gfx::PassAttachments{}
					.Attach({ .resource = *state_positions_texture_.resource_ }, D3D12_RESOURCE_STATE_COPY_DEST)
				);
			}

			particle_depth_pass_ = device->graph_.AddSubsequentPass(Gfx::PassAttachments{}
				.Attach({ .resource = *state_positions_texture_.resource_ }, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
				.Attach({ .resource = *depth_target_.resource_ }, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
				.Attach({ .resource = *color_target->resource_ }, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
			);
		}

		void Render(Gfx::Device * device, Gfx::Encoder * encoder, Rendering::Viewport * viewport, Gfx::Resource * color_texture) {
			if (frame_data_queue_.Size() > max_frames_queued_) {
				frame_data_queue_.RemoveAt(0);
				// TODO: fancier ahead-of-time waitables
				//frame_data_queue_.RemoveAt(0).waitable_.Wait();
			}

			FrameData frame_data;

			if(updates_.Size()) {
				encoder->SetPass(update_positions_pass_);
				update_positions_pass_ = nullptr;

			
				frame_data.upload_buffer_ = device->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, updates_.Size() * 256, DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ);
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

			frame_data.page_buffer_ = device->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, active_pages_.Size() * sizeof(i32), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ);

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
				device->device_->CreateShaderResourceView(*frame_data.page_buffer_.resource_, &srv_desc, encoder->ReserveComputeSlot(Gfx::DescriptorType::SRV, 0));
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
				device->device_->CreateShaderResourceView(*state_positions_texture_.resource_, &srv_desc, encoder->ReserveComputeSlot(Gfx::DescriptorType::SRV, 1));
			}

			{
				//depth
				D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{
					.Format = DXGI_FORMAT_R32_UINT,
					.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D,
					.Texture2D = {}
				};
				device->device_->CreateUnorderedAccessView(*depth_target_.resource_, nullptr, &uav_desc, encoder->ReserveComputeSlot(Gfx::DescriptorType::UAV, 0));
			}
			{
				//color
				D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{
					.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
					.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D,
					.Texture2D = {}
				};
				device->device_->CreateUnorderedAccessView(*color_texture->resource_, nullptr, &uav_desc, encoder->ReserveComputeSlot(Gfx::DescriptorType::UAV, 1));
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

			frame_data.constant_buffers_.PushBackRvalueRef(std::move(device->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, AlignedForward(static_cast<i32>(sizeof(frame_constants)), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ)));

			void* cb_dst = nullptr;
			verify_hr(frame_data.constant_buffers_.Last().resource_->Map(0, nullptr, &cb_dst));
			memcpy(cb_dst, &frame_constants, sizeof(frame_constants));
			frame_data.constant_buffers_.Last().resource_->Unmap(0, nullptr);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc{
				.BufferLocation = frame_data.constant_buffers_.Last().resource_->GetGPUVirtualAddress(),
				.SizeInBytes = AlignedForward(static_cast<u32>(sizeof(frame_constants)), static_cast<u32>(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT))
			};
			device->device_->CreateConstantBufferView(&cbv_desc, encoder->ReserveComputeSlot(Gfx::DescriptorType::CBV, 0));

			encoder->SetPass(particle_depth_pass_);
			encoder->GetCmdList()->SetPipelineState(*depth_pass_pipeline_.pipeline_);
			encoder->SetComputeDescriptors();
			particle_depth_pass_ = nullptr;
			encoder->GetCmdList()->Dispatch(static_cast<u32>(active_pages_.Size() * PAGE_SIZE), 1, 1);

			frame_data_queue_.PushBackRvalueRef(std::move(frame_data));
		}
	};

	Vector3 PolygonGenerator::GeneratePoint() {
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

void InitImgui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
}

struct ImGuiRenderer : private Pinned<ImGuiRenderer> {
	Gfx::Resource font_texture_;
	Gfx::Device* device_ = nullptr;
	Gfx::Pipeline pipeline_;

	struct FrameData {
		Gfx::Resource vertex_buffer_;
		Gfx::Resource index_buffer_;
		Array<Gfx::Resource> constant_buffers_;
		Gfx::Waitable waitable_;
	};

	i32 max_frames_queued_ = 3;
	Array<FrameData> frame_data_queue_;

	struct ViewportData {
	};

	ViewportData main_viewport_;

	void Init(Gfx::Device* device) {
		device_ = device;

		ImGuiIO& io = ImGui::GetIO();
		io.BackendRendererName = "imgui_impl_playground";
		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
		//io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;

		ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		main_viewport->RendererUserData = &main_viewport_;

		// Setup back-end capabilities flags
		//io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
		//if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		//	ImGui_ImplDX12_InitPlatformInterface();

		CreateFontsTexture();
		CreatePipeline();
	}

	void Shutdown() {
		ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		main_viewport->RendererUserData = nullptr;

		frame_data_queue_.Clear();
	}

	void CreateFontsTexture() {
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

		font_texture_ = device_->CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, { width, height }, DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);

		i32 upload_pitch = AlignedForward(width * 4, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
		D3D12_RESOURCE_DESC desc = font_texture_.resource_->GetDesc();
		Gfx::Resource upload_buffer = device_->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, width * upload_pitch, DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ);

		u8* mapped_memory = nullptr;
		verify_hr(upload_buffer.resource_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_memory)));

		for (i64 r = 0; r < height; r++) {
			memcpy(mapped_memory + r * upload_pitch, pixels + r * width * 4, width * 4);
		}

		upload_buffer.resource_->Unmap(0, nullptr);

		Gfx::Encoder encoder = device_->CreateEncoder();
		D3D12_TEXTURE_COPY_LOCATION copy_src{
			.pResource = *upload_buffer.resource_,
			.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
			.PlacedFootprint = {
				.Footprint = {
					.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
					.Width = static_cast<u32>(width),
					.Height = static_cast<u32>(height),
					.Depth = 1,
					.RowPitch = static_cast<u32>(upload_pitch)
				}
			}
		};
		D3D12_TEXTURE_COPY_LOCATION copy_dst{
			.pResource = *font_texture_.resource_,
			.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
			.SubresourceIndex = 0
		};
		encoder.GetCmdList()->CopyTextureRegion(&copy_dst, 0, 0, 0, &copy_src, nullptr);

		Gfx::Pass* transition_to_readable_pass = device_->graph_.AddSubsequentPass(Gfx::PassAttachments{}.Attach({ .resource = *font_texture_.resource_ }, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		encoder.SetPass(transition_to_readable_pass);

		encoder.Submit();
		device_->GetWaitable().Wait();

		// Store our identifier
		//static_assert(sizeof(ImTextureID) >= sizeof(g_hFontSrvGpuDescHandle.ptr), "Can't pack descriptor handle into TexID, 32-bit not supported yet.");
		//io.Fonts->TexID = (ImTextureID)g_hFontSrvGpuDescHandle.ptr;
	}

	void CreatePipeline() {
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc{};
		pso_desc.NodeMask = 1;
		pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pso_desc.pRootSignature = *device_->root_signature_;
		pso_desc.SampleMask = UINT_MAX;
		pso_desc.NumRenderTargets = 1;
		pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		pso_desc.SampleDesc.Count = 1;
		pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		Gfx::ShaderBlob* vs_bytecode;
		{
			static const char* vertex_shader =
				"cbuffer vertexBuffer : register(b0) \
            {\
              float4x4 ProjectionMatrix; \
            };\
            struct VS_INPUT\
            {\
              float2 pos : POSITION;\
              float4 col : COLOR0;\
              float2 uv  : TEXCOORD0;\
            };\
            \
            struct PS_INPUT\
            {\
              float4 pos : SV_POSITION;\
              float4 col : COLOR0;\
              float2 uv  : TEXCOORD0;\
            };\
            \
            PS_INPUT main(VS_INPUT input)\
            {\
              PS_INPUT output;\
              output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\
              output.col = input.col;\
              output.uv  = input.uv;\
              return output;\
            }";

			Gfx::ShaderBlob* vs_blob = Gfx::Wrap(vertex_shader, strlen(vertex_shader));
			vs_bytecode = Gfx::CompileShader(vs_blob, nullptr, L"vs", L"main", L"vs_6_0");
			pso_desc.VS = { vs_bytecode->GetBufferPointer(), vs_bytecode->GetBufferSize() };

			vs_blob->Release();
		}

		// Create the input layout
		static D3D12_INPUT_ELEMENT_DESC local_layout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (UINT)IM_OFFSETOF(ImDrawVert, pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (UINT)IM_OFFSETOF(ImDrawVert, uv),  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (UINT)IM_OFFSETOF(ImDrawVert, col), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		pso_desc.InputLayout = { local_layout, 3 };

		Gfx::ShaderBlob* ps_bytecode;
		{
			static const char* pixel_shader =
				"struct PS_INPUT\
            {\
              float4 pos : SV_POSITION;\
              float4 col : COLOR0;\
              float2 uv  : TEXCOORD0;\
            };\
            SamplerState sampler0 : register(s0);\
            Texture2D texture0 : register(t0);\
            \
            float4 main(PS_INPUT input) : SV_Target\
            {\
              float4 out_col = input.col * texture0.Sample(sampler0, input.uv); \
              return out_col; \
            }";

			Gfx::ShaderBlob* ps_blob = Gfx::Wrap(pixel_shader, strlen(pixel_shader));
			ps_bytecode = Gfx::CompileShader(ps_blob, nullptr, L"ps", L"main", L"ps_6_0");
			pso_desc.PS = { ps_bytecode->GetBufferPointer(), ps_bytecode->GetBufferSize() };

			ps_blob->Release();
		}

		// Create the blending setup
		{
			D3D12_BLEND_DESC& desc = pso_desc.BlendState;
			desc.AlphaToCoverageEnable = false;
			desc.RenderTarget[0].BlendEnable = true;
			desc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			desc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
			desc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
			desc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
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
			desc.DepthEnable = false;
			desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
			desc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
			desc.StencilEnable = false;
			desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
			desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
			desc.BackFace = desc.FrontFace;
		}

		verify_hr(device_->device_->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(pipeline_.pipeline_.InitAddress())));

		vs_bytecode->Release();
		ps_bytecode->Release();
	}

	void SetupRenderState(ImDrawData* draw_data, Gfx::Encoder* encoder, FrameData* frame_data) {
		float L = draw_data->DisplayPos.x;
		float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
		float T = draw_data->DisplayPos.y;
		float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
		float mvp[4][4] =
		{
			{ 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
			{ 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
			{ 0.0f,         0.0f,           0.5f,       0.0f },
			{ (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
		};

		// Setup viewport
		D3D12_VIEWPORT vp;
		memset(&vp, 0, sizeof(D3D12_VIEWPORT));
		vp.Width = draw_data->DisplaySize.x;
		vp.Height = draw_data->DisplaySize.y;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = vp.TopLeftY = 0.0f;
		encoder->GetCmdList()->RSSetViewports(1, &vp);

		// Bind shader and vertex buffers
		unsigned int stride = sizeof(ImDrawVert);
		unsigned int offset = 0;
		D3D12_VERTEX_BUFFER_VIEW vbv;
		memset(&vbv, 0, sizeof(D3D12_VERTEX_BUFFER_VIEW));
		vbv.BufferLocation = frame_data->vertex_buffer_.resource_->GetGPUVirtualAddress() + offset;
		vbv.SizeInBytes = draw_data->TotalVtxCount * stride;
		vbv.StrideInBytes = stride;
		encoder->GetCmdList()->IASetVertexBuffers(0, 1, &vbv);
		D3D12_INDEX_BUFFER_VIEW ibv;
		memset(&ibv, 0, sizeof(D3D12_INDEX_BUFFER_VIEW));
		ibv.BufferLocation = frame_data->index_buffer_.resource_->GetGPUVirtualAddress();
		ibv.SizeInBytes = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
		ibv.Format = sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		encoder->GetCmdList()->IASetIndexBuffer(&ibv);
		encoder->GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		encoder->GetCmdList()->SetPipelineState(*pipeline_.pipeline_);

		frame_data->constant_buffers_.PushBackRvalueRef(std::move(device_->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, AlignedForward(64, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ)));

		void* cb_dst = nullptr;
		verify_hr(frame_data->constant_buffers_.Last().resource_->Map(0, nullptr, &cb_dst));
		memcpy(cb_dst, &mvp[0][0], 64);
		frame_data->constant_buffers_.Last().resource_->Unmap(0, nullptr);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc{
			.BufferLocation = frame_data->constant_buffers_.Last().resource_->GetGPUVirtualAddress(),
			.SizeInBytes = static_cast<u32>(AlignedForward(64, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT))
		};
		device_->device_->CreateConstantBufferView(&cbv_desc, encoder->ReserveGraphicsSlot(Gfx::DescriptorType::CBV, 0));

		// Setup blend factor
		const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
		encoder->GetCmdList()->OMSetBlendFactor(blend_factor);
	}

	void RenderDrawData(ImDrawData* draw_data, Gfx::Encoder* encoder, D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle) {
		if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
			return;

		if (draw_data->TotalVtxCount == 0 || draw_data->TotalIdxCount == 0) {
			return;
		}

		if (frame_data_queue_.Size() > max_frames_queued_) {
			frame_data_queue_.RemoveAt(0);
			// TODO: fancier ahead-of-time waitables
			//frame_data_queue_.RemoveAt(0).waitable_.Wait();
		}

		ViewportData* render_data = (ViewportData*)draw_data->OwnerViewport->RendererUserData;

		encoder->GetCmdList()->OMSetRenderTargets(1, &rtv_handle, false, nullptr);

		FrameData frame_data;
		frame_data.vertex_buffer_ = device_->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, draw_data->TotalVtxCount * sizeof(ImDrawVert), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ);
		frame_data.index_buffer_ = device_->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, draw_data->TotalIdxCount * sizeof(ImDrawIdx), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ);

		ImDrawVert* vtx_dst = nullptr;
		ImDrawIdx* idx_dst = nullptr;
		verify_hr(frame_data.vertex_buffer_.resource_->Map(0, nullptr, reinterpret_cast<void**>(&vtx_dst)));
		verify_hr(frame_data.index_buffer_.resource_->Map(0, nullptr, reinterpret_cast<void**>(&idx_dst)));

		for (i32 n = 0, N = draw_data->CmdListsCount; n < N; n++) {
			const ImDrawList* cmd_list = draw_data->CmdLists[n];

			memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));

			vtx_dst += cmd_list->VtxBuffer.Size;
			idx_dst += cmd_list->IdxBuffer.Size;
		}

		frame_data.vertex_buffer_.resource_->Unmap(0, nullptr);
		frame_data.index_buffer_.resource_->Unmap(0, nullptr);

		SetupRenderState(draw_data, encoder, &frame_data);

		int global_vtx_offset = 0;
		int global_idx_offset = 0;
		ImVec2 clip_off = draw_data->DisplayPos;
		for (int n = 0; n < draw_data->CmdListsCount; n++) {
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback != nullptr) {
					if (pcmd->UserCallback == ImDrawCallback_ResetRenderState) {
						SetupRenderState(draw_data, encoder, &frame_data);
					}
					else {
						pcmd->UserCallback(cmd_list, pcmd);
					}
				}
				else {
					// Apply Scissor, Bind texture, Draw
					const D3D12_RECT r = { (LONG)(pcmd->ClipRect.x - clip_off.x), (LONG)(pcmd->ClipRect.y - clip_off.y), (LONG)(pcmd->ClipRect.z - clip_off.x), (LONG)(pcmd->ClipRect.w - clip_off.y) };
					//ctx->SetGraphicsRootDescriptorTable(1, *(D3D12_GPU_DESCRIPTOR_HANDLE*)&pcmd->TextureId);
					D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{
						.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
						.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
						.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
						.Texture2D = {.MipLevels = 1 }
					};
					device_->device_->CreateShaderResourceView(*font_texture_.resource_, &srv_desc, encoder->ReserveGraphicsSlot(Gfx::DescriptorType::SRV, 0));
					encoder->GetCmdList()->RSSetScissorRects(1, &r);
					encoder->SetGraphicsDescriptors();
					encoder->GetCmdList()->DrawIndexedInstanced(pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
				}
			}
			global_idx_offset += cmd_list->IdxBuffer.Size;
			global_vtx_offset += cmd_list->VtxBuffer.Size;
		}

		frame_data_queue_.PushBackRvalueRef(std::move(frame_data));
	}
};

int main(int argc, char** argv)
{
	Gfx::Device device;

	InitImgui();

	ImGuiRenderer imgui_renderer;
	imgui_renderer.Init(&device);

	Rendering::ImmediateModeShapeRenderer shape_renderer;
	shape_renderer.Init(&device);

	Particle::PolygonGenerator particle_generator;
	particle_generator.Init(&device, 100000, 1000.f, 10.f);
	particle_generator.vertices_.PushBack({-2, 5, -2});
	particle_generator.vertices_.PushBack({2, 5, -2});
	particle_generator.vertices_.PushBack({-2, 5, 2});
	particle_generator.vertices_.PushBack({2, 5, -2});
	particle_generator.vertices_.PushBack({2, 5, 2});
	particle_generator.vertices_.PushBack({-2, 5, 2});

	particle_generator.CreatePipelines(&device);

	// TODO: who should own backbuffers num?
	// Device, swapchain?
	i32 backbuffers_num = 3;

	// TODO: this should be one call creating (device, window, swapchain) tuple?
	// one device can have many (window, swapchain) pairs
	Os::Window* window = Os::CreateOsWindow({ 1920, 1080 });
	Gfx::Swapchain* window_swapchain = device.CreateSwapchain(window, backbuffers_num);
	// this is here, because WndProc needs the mappings ready
	window->Init();
	ImGui_ImplWin32_Init(window->hwnd_);

	Gfx::ShaderBlob* compiled_shader = Gfx::CompileShaderFromFile(
		L"../data/shader.hlsl",
		L"../data/shader.hlsl",
		L"main",
		L"cs_6_0"
	);

	D3D12_SHADER_BYTECODE CS;
	CS.pShaderBytecode = compiled_shader->GetBufferPointer();
	CS.BytecodeLength = compiled_shader->GetBufferSize();

	Gfx::Pipeline cs = device.CreateComputePipeline(CS);
	compiled_shader->Release();

	struct ScreenResources {
		Gfx::Resource random_access_texture;
		Vector2i resolution;
	};

	ScreenResources screen_resources{ .resolution = window->resolution_ };
	screen_resources.random_access_texture = device.CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, window->resolution_, DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	int frames_ctr = 3;

	Array<Gfx::Waitable> frame_waitables;

	Rendering::Viewport main_viewport{};

	main_viewport.camera_look_at = { 0, 0, 0 };
	main_viewport.camera_position = { 5, 5, -5 };
	main_viewport.camera_up = { 0, 1, 0 };
	main_viewport.resolution = window->resolution_;
	main_viewport.fov_y = Magnum::Math::Constants<float>::piHalf();
	main_viewport.near_plane = 0.1f;
	main_viewport.far_plane = 1000.f;

	window->RunMessageLoop([
		&window,
			&device,
			&window_swapchain,
			&screen_resources,
			&cs,
			backbuffers_num,
			&frames_ctr,
			&frame_waitables,
			&imgui_renderer,
			&shape_renderer,
			&main_viewport,
			&particle_generator
	]() {
		i32 current_backbuffer_index = window_swapchain->swapchain_->GetCurrentBackBufferIndex();

		if (screen_resources.resolution != window->resolution_) {
			screen_resources.random_access_texture = device.CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, window->resolution_, DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			screen_resources.resolution = window->resolution_;
			main_viewport.resolution = window->resolution_;
		}

		particle_generator.AllocateScreenTextures(&device, main_viewport.resolution);

		f32 camera_sensivity = 0.1f;
		if(window->user_input_.keys_down_['W']) {
			main_viewport.TranslateCamera(main_viewport.GetLookToVector() * camera_sensivity);
		}
		if(window->user_input_.keys_down_['S']) {
			main_viewport.TranslateCamera(-main_viewport.GetLookToVector() * camera_sensivity);
		}
		if(window->user_input_.keys_down_['A']) {
			main_viewport.TranslateCamera(-main_viewport.GetRightVector() * camera_sensivity);
		}
		if(window->user_input_.keys_down_['D']) {
			main_viewport.TranslateCamera(main_viewport.GetRightVector() * camera_sensivity);
		}

		ImGuiIO& io = ImGui::GetIO();
		
		if(!io.WantCaptureMouse && window->user_input_.mouse_down_[0]) {
			f32 delta_x = io.MouseDelta.x / static_cast<f32>(window->resolution_.x());
			f32 delta_y = -io.MouseDelta.y / static_cast<f32>(window->resolution_.y());

			// 
			Vector4 clip_space_point_to{delta_x * 2.f, delta_y * 2.f, 1.f, 1.f};

			Vector4 world_point_to = main_viewport.inv_view_projection_matrix * clip_space_point_to;
			world_point_to = (world_point_to).normalized();

			main_viewport.camera_look_at = main_viewport.camera_position + world_point_to.xyz();
		}

		particle_generator.Tick(io.DeltaTime);

		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		{
			ImGui::Begin("Particles");

			ImGui::Text("Time: %f ms", io.DeltaTime * 1000.f);
			ImGui::Text("Particles num: %d", particle_generator.NumParticles());
			ImGui::Text("Particles pages: %d", particle_generator.active_pages_.Size());

			ImGui::End();
		}



		main_viewport.view_matrix = Rendering::LookAtLh(main_viewport.camera_look_at, main_viewport.camera_position, main_viewport.camera_up);
		main_viewport.inv_view_matrix = Rendering::InverseLookAtLh(main_viewport.camera_look_at, main_viewport.camera_position, main_viewport.camera_up);
		main_viewport.projection_matrix = Rendering::PerspectiveFovLh(main_viewport.GetAspectRatio(), main_viewport.fov_y, main_viewport.near_plane, main_viewport.far_plane);
		main_viewport.inv_projection_matrix = Rendering::InversePerspectiveFovLh(main_viewport.GetAspectRatio(), main_viewport.fov_y, main_viewport.near_plane, main_viewport.far_plane);
		main_viewport.view_projection_matrix = main_viewport.projection_matrix * main_viewport.view_matrix;
		main_viewport.inv_view_projection_matrix = main_viewport.inv_view_matrix * main_viewport.inv_projection_matrix;

		ID3D12Resource* current_backbuffer = *window_swapchain->backbuffers_[current_backbuffer_index];

		Gfx::Pass* shader_execution_pass = device.graph_.AddSubsequentPass(Gfx::PassAttachments{}
			.Attach({ .resource = *screen_resources.random_access_texture.resource_ }, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		);

		particle_generator.AddPassesToGraph(&device, &screen_resources.random_access_texture);

		Gfx::Pass* render_ui_pass = device.graph_.AddSubsequentPass(Gfx::PassAttachments{}
			.Attach({ .resource = *screen_resources.random_access_texture.resource_ }, D3D12_RESOURCE_STATE_RENDER_TARGET)
		);

		Gfx::Pass* copy_to_backbuffer_pass = device.graph_.AddSubsequentPass(Gfx::PassAttachments{}
			.Attach({ .resource = *screen_resources.random_access_texture.resource_ }, D3D12_RESOURCE_STATE_COPY_SOURCE)
			.Attach({ .resource = current_backbuffer }, D3D12_RESOURCE_STATE_COPY_DEST)
		);

		Gfx::Pass* present_pass = device.graph_.AddSubsequentPass(Gfx::PassAttachments{}
			.Attach({ .resource = current_backbuffer }, D3D12_RESOURCE_STATE_PRESENT)
		);

		Gfx::Encoder encoder = device.CreateEncoder();

		ImGui::Render();

		encoder.SetPass(shader_execution_pass);
		ID3D12GraphicsCommandList* cmd_list = encoder.GetCmdList();
		cmd_list->SetPipelineState(*cs.pipeline_);
		D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D,
			.Texture2D = {}
		};
		device.device_->CreateUnorderedAccessView(*screen_resources.random_access_texture.resource_, nullptr, &uav_desc, encoder.ReserveComputeSlot(Gfx::DescriptorType::UAV, 0));
		encoder.SetComputeDescriptors();
		cmd_list->Dispatch((window->resolution_.x() + 7) / 8, (window->resolution_.y() + 3) / 4, 1);

		D3D12_RESOURCE_BARRIER barrier {
			.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV,
			.UAV = { .pResource = *screen_resources.random_access_texture.resource_ }
		};
		cmd_list->ResourceBarrier(1, &barrier);
		particle_generator.Render(&device, &encoder, &main_viewport, &screen_resources.random_access_texture);

		encoder.SetPass(render_ui_pass);

		D3D12_RENDER_TARGET_VIEW_DESC rtv_desc{
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
			.Texture2D = {}
		};

		D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = device.rtvs_descriptor_heap_.heap_->GetCPUDescriptorHandleForHeapStart();
		rtv_handle.ptr += device.rtvs_descriptor_heap_.AllocateTable(1) * device.rtvs_descriptor_heap_.increment_;
		device.device_->CreateRenderTargetView(*screen_resources.random_access_texture.resource_, &rtv_desc, rtv_handle);
		shape_renderer.Render(&encoder, &main_viewport, rtv_handle);
		imgui_renderer.RenderDrawData(ImGui::GetDrawData(), &encoder, rtv_handle);

		encoder.SetPass(copy_to_backbuffer_pass);
		cmd_list->CopyResource(current_backbuffer, *screen_resources.random_access_texture.resource_);

		encoder.SetPass(present_pass);

		// we should wait for the presenting being done before we access that backbuffer again?
		if (frame_waitables.Size() == backbuffers_num) {
			frame_waitables.RemoveAt(0).Wait();
		}

		encoder.Submit();

		verify_hr(window_swapchain->swapchain_->Present(1, 0));
		device.AdvanceFence();

		frame_waitables.PushBack(device.GetWaitable());

		current_backbuffer_index = (current_backbuffer_index + 1) % backbuffers_num;

	});

	device.GetWaitable().Wait();

	ImGui_ImplWin32_Shutdown();
	imgui_renderer.Shutdown();
	shape_renderer.Shutdown();
	ImGui::DestroyContext();

	return 0;
}
