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
#include "particles.h"

#include <math.h>

using namespace Containers;
using namespace IO;

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

namespace Rendering {
struct SphereTracer {
		Gfx::Device* device_ = nullptr;
		Gfx::Pipeline pipeline_;

		struct FrameData {
			Containers::Array<Gfx::Resource> constant_buffers_;
			Gfx::Waitable waitable_;
		};

		Containers::Array<FrameData> frame_data_queue_;

		void Init(Gfx::Device* device);
		void Render(Gfx::Encoder* encoder, Viewport* viewport, D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle);
		
		void _CreatePipeline() {
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
				vs_bytecode = Gfx::CompileShaderFromFile(L"../data/sphere.hlsl", L"../data/sphere.hlsl", L"VsMain", L"vs_6_0");
				pso_desc.VS = { vs_bytecode->GetBufferPointer(), vs_bytecode->GetBufferSize() };
			}
			
			Gfx::ShaderBlob* ps_bytecode;
			{
				ps_bytecode = Gfx::CompileShaderFromFile(L"../data/sphere.hlsl", L"../data/sphere.hlsl", L"PsMain", L"ps_6_0");
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
				desc.AntialiasedLineEnable = FALSE;
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
	};

	void SphereTracer::Init(Gfx::Device* device) {
		device_ = device;

		_CreatePipeline();
	}

	void SphereTracer::Render(Gfx::Encoder* encoder, Viewport* viewport, D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle) {
		while(frame_data_queue_.Size() && frame_data_queue_.First().waitable_.IsDone()) {
			frame_data_queue_.RemoveAt(0);
		}

		encoder->GetCmdList()->OMSetRenderTargets(1, &rtv_handle, false, nullptr);

		D3D12_VIEWPORT vp{
			.Width = static_cast<float>(viewport->resolution.x()),
			.Height = static_cast<float>(viewport->resolution.y()),
			.MinDepth = 0.f,
			.MaxDepth = 1.f
		};
		encoder->GetCmdList()->RSSetViewports(1, &vp);

		FrameData frame_data;

		struct FrameConstants {
			Vector2i resolution;
			Vector2 inv_resolution;
			Matrix4 view_projection_matrix;
			Matrix4 inv_view_projection_matrix;
			Matrix4 inv_view_matrix;
		};

		encoder->GetCmdList()->IASetVertexBuffers(0, 1, nullptr);
		encoder->GetCmdList()->IASetIndexBuffer(nullptr);
		encoder->GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		encoder->GetCmdList()->SetPipelineState(*pipeline_.pipeline_);

		frame_data.constant_buffers_.PushBackRvalueRef(std::move(device_->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, AlignedForward(static_cast<i32>(sizeof(FrameConstants)), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ)));

		FrameConstants constants;
		constants.resolution = viewport->resolution;
		constants.inv_resolution = 1.f / Vector2(constants.resolution);
		constants.view_projection_matrix = viewport->view_projection_matrix;
		constants.inv_view_projection_matrix = viewport->inv_view_projection_matrix;
		constants.inv_view_matrix = viewport->inv_view_matrix;

		void* cb_dst = nullptr;
		verify_hr(frame_data.constant_buffers_.Last().resource_->Map(0, nullptr, &cb_dst));
		memcpy(cb_dst, &constants, sizeof(FrameConstants));
		frame_data.constant_buffers_.Last().resource_->Unmap(0, nullptr);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc{
			.BufferLocation = frame_data.constant_buffers_.Last().resource_->GetGPUVirtualAddress(),
			.SizeInBytes = static_cast<u32>(AlignedForward(64, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT))
		};
		device_->device_->CreateConstantBufferView(&cbv_desc, encoder->ReserveGraphicsSlot(Gfx::DescriptorType::CBV, 0));

		encoder->SetGraphicsDescriptors();

		const D3D12_RECT r = { 0, 0, viewport->resolution.x(), viewport->resolution.y() };
		encoder->GetCmdList()->RSSetScissorRects(1, &r);
		encoder->GetCmdList()->DrawInstanced(3, 1, 0, 0);

		frame_data.waitable_ = encoder->GetWaitable();

		frame_data_queue_.PushBackRvalueRef(std::move(frame_data));
	}
}

namespace Geometry {
	// http://jcgt.org/published/0002/02/05/paper.pdf
	struct AxisBounds {
		Vector3 L;
		Vector3 U;
	};

	f32 Square(f32 x) {
		return x * x;
	}

	AxisBounds GetBoundForAxis(Vector3 a, Vector3 C, float r, float near_z) {
		const Vector2 c(dot(a, C), C.z()); //C in the a-z frame
		Vector2 bounds[2]; //In the a-z reference frame
		const float t_squared = dot(c, c) - Square(r);
		const bool camera_inside_sphere = (t_squared <= 0); 
		//(cos, sin) of angle theta between c and a tangent vector
		Vector2 v = camera_inside_sphere ?
			Vector2(0.0f, 0.0f) :
			Vector2(sqrt(t_squared), r) / c.length();
	
		//Does the near plane intersect the sphere?
		const bool clip_sphere = (c.y() - r <= near_z);
		//Square root of the discriminant; NaN (and unused)
		//if the camera is in the sphere
		float k = sqrt(Square(r) - Square(c.y() - near_z));
		for (int i = 0; i < 2; ++i) {
			if (! camera_inside_sphere) {
				bounds[i] = Matrix2x2{ Vector2{v.x(), -v.y()}, Vector2{v.y(), v.x()} } * c * v.x();
			}
			const bool clip_bound = camera_inside_sphere || (bounds[i].y() < near_z);
			if (clip_sphere && clip_bound) {
				bounds[i] = Vector2(c.x() + k, near_z);
			}
			//Set up for the lower bound
			v.y() = -v.y();  k = -k;
		}

		AxisBounds result = { .L = { bounds[1].x() * a }, .U = { bounds[0].x() * a }};
		result.L.z() = bounds[1].y();
		result.U.z() = bounds[0].y();
		return result;
	}

	struct AABox2D {
		Vector2 vec_min;
		Vector2 vec_max;

		f32 Area() const;
		static AABox2D Xyxy(Vector2 xy0, Vector2 xy1) {
			using namespace Algorithms;
			return { .vec_min = {min(xy0.x(), xy1.x()), min(xy0.y(), xy1.y())},
				.vec_max = {max(xy0.x(), xy1.x()), max(xy0.y(), xy1.y())}};
		}
	};

	f32 AABox2D::Area() const {
		return (vec_max - vec_min).product();
	}

	Vector3 Project(Matrix4x4 P, Vector3 Q) {
		Vector4 v = P * Vector4{Q, 1.f};
		return v.xyz() / v.w();
	}

	AABox2D GetAxisAlignedBoundingBox(Vector3 C, float r, float nearZ, Matrix4x4 P) {
		const auto& [left, right] = GetBoundForAxis({1,0,0}, C, r, nearZ);
		const auto& [bottom, top] = GetBoundForAxis({0,1,0}, C, r, nearZ);

		return AABox2D::Xyxy({Project(P, left).x(), Project(P, bottom).y()}, {Project(P, right).x(), Project(P, top).y()});
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

	Rendering::PolygonParticleGenerator particle_generator;
	particle_generator.Init(&device, 100000, 1000.f, 10.f);
	particle_generator.vertices_.PushBack({-2, 5, -2});
	particle_generator.vertices_.PushBack({2, 5, -2});
	particle_generator.vertices_.PushBack({-2, 5, 2});
	particle_generator.vertices_.PushBack({2, 5, -2});
	particle_generator.vertices_.PushBack({2, 5, 2});
	particle_generator.vertices_.PushBack({-2, 5, 2});

	Rendering::SphereTracer sphere_tracer;
	sphere_tracer.Init(&device);

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
			&sphere_tracer,
			&main_viewport,
			&particle_generator
	]() {
		i32 current_backbuffer_index = window_swapchain->swapchain_->GetCurrentBackBufferIndex();

		if (screen_resources.resolution != window->resolution_) {
			screen_resources.random_access_texture = device.CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, window->resolution_, DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			screen_resources.resolution = window->resolution_;
			main_viewport.resolution = window->resolution_;
		}

		particle_generator.AllocateScreenTextures(main_viewport.resolution);

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

			main_viewport.camera_look_at = main_viewport.camera_position + world_point_to.xyz().normalized();
		}

		//particle_generator.Tick(io.DeltaTime);

		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		main_viewport.view_matrix = Rendering::LookAtLh(main_viewport.camera_look_at, main_viewport.camera_position, main_viewport.camera_up);
		main_viewport.inv_view_matrix = Rendering::InverseLookAtLh(main_viewport.camera_look_at, main_viewport.camera_position, main_viewport.camera_up);
		main_viewport.projection_matrix = Rendering::PerspectiveFovLh(main_viewport.GetAspectRatio(), main_viewport.fov_y, main_viewport.near_plane, main_viewport.far_plane);
		main_viewport.inv_projection_matrix = Rendering::InversePerspectiveFovLh(main_viewport.GetAspectRatio(), main_viewport.fov_y, main_viewport.near_plane, main_viewport.far_plane);
		main_viewport.view_projection_matrix = main_viewport.projection_matrix * main_viewport.view_matrix;
		main_viewport.inv_view_projection_matrix = main_viewport.inv_view_matrix * main_viewport.inv_projection_matrix;


		Rendering::ShapeVertex vertices[] = {
			{.position = {0, 0, 0}, .colour = {255, 0, 0, 1}},
			{.position = {1, 0, 0}, .colour = {255, 0, 0, 1}},
			{.position = {0, 0, 0}, .colour = {0, 255, 0, 1}},
			{.position = {0, 1, 0}, .colour = {0, 255, 0, 1}},
			{.position = {0, 0, 0}, .colour = {0, 0, 255, 1}},
			{.position = {0, 0, 1}, .colour = {0, 0, 255, 1}},
		};
		shape_renderer.vertices_.Append(vertices, _countof(vertices));

		Geometry::AABox2D bb = Geometry::GetAxisAlignedBoundingBox(Vector4{main_viewport.view_matrix * Vector4{0,0,0,1}}.xyz(), 1.f, main_viewport.near_plane, main_viewport.projection_matrix);
		
		{
			ImGui::Begin("Particles");

			ImGui::Text("Time: %f ms", io.DeltaTime * 1000.f);
			ImGui::Text("Particles num: %d", particle_generator.NumParticles());
			ImGui::Text("Particles pages: %d", particle_generator.active_pages_.Size());
			ImGui::Text("BB area: %f", bb.Area());

			ImGui::End();
		}

		Vector4 A = main_viewport.inv_view_projection_matrix * Vector4{bb.vec_min.x(), bb.vec_min.y(), 0.f, 1.f};
		Vector4 B = main_viewport.inv_view_projection_matrix * Vector4{bb.vec_min.x(), bb.vec_max.y(), 0.f, 1.f};
		Vector4 C = main_viewport.inv_view_projection_matrix * Vector4{bb.vec_max.x(), bb.vec_max.y(), 0.f, 1.f};
		Vector4 D = main_viewport.inv_view_projection_matrix * Vector4{bb.vec_max.x(), bb.vec_min.y(), 0.f, 1.f};
		A /= A.w();
		B /= B.w();
		C /= C.w();
		D /= D.w();
		
		shape_renderer.AddLine(A.xyz(), B.xyz(), {255, 128, 128, 255});
		shape_renderer.AddLine(B.xyz(), C.xyz(), {255, 128, 128, 255});
		shape_renderer.AddLine(C.xyz(), D.xyz(), {255, 128, 128, 255});
		shape_renderer.AddLine(A.xyz(), D.xyz(), {255, 128, 128, 255});

		ID3D12Resource* current_backbuffer = *window_swapchain->backbuffers_[current_backbuffer_index];

		Gfx::Pass* shader_execution_pass = device.graph_.AddSubsequentPass(Gfx::PassAttachments{}
			.Attach({ .resource = *screen_resources.random_access_texture.resource_ }, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		);

		particle_generator.AddPassesToGraph(&screen_resources.random_access_texture);

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
		particle_generator.Render(&encoder, &main_viewport, &screen_resources.random_access_texture);

		encoder.SetPass(render_ui_pass);

		D3D12_RENDER_TARGET_VIEW_DESC rtv_desc{
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
			.Texture2D = {}
		};

		D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = device.rtvs_descriptor_heap_.heap_->GetCPUDescriptorHandleForHeapStart();
		rtv_handle.ptr += device.rtvs_descriptor_heap_.AllocateTable(1) * device.rtvs_descriptor_heap_.increment_;
		device.device_->CreateRenderTargetView(*screen_resources.random_access_texture.resource_, &rtv_desc, rtv_handle);
		sphere_tracer.Render(&encoder, &main_viewport, rtv_handle);
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
