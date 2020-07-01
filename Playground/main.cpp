#include "pch.h"

#include "gfx.h"
#include <fmt/format.h>
#include "array.h"
#include "assertions.h"
#include "os.h"
#include "files.h"

#include "shader.h"

#include <imgui/imgui.h>
#include "imgui_impl_win32.h"

#include "rendering.h"
#include "imgui_backend.h"
#include "shapes.h"

#include <math.h>

using namespace Containers;
using namespace IO;

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
			while(frame_data_queue_.Size() && frame_data_queue_.First().waitable_.IsDone()) {
				frame_data_queue_.RemoveAt(0);
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

			frame_data.waitable_ = encoder->GetWaitable();

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

int main(int argc, char** argv)
{
	Gfx::Device device;

	InitImgui();

	Rendering::ImGuiRenderer imgui_renderer;
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
		device.RecycleResources();

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
