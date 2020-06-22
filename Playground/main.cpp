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

template<typename T>
T aligned_forward(T v, T a) {
	return (v + (a - 1)) & ~(a - 1);
}

struct ImGuiRenderer : private Pinned<ImGuiRenderer> {
	Gfx::Resource font_texture_;
	Gfx::Device* device_ = nullptr;
	Gfx::Pipeline pipeline_;

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
	}

	void CreateFontsTexture() {
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

		font_texture_ = device_->CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, { width, height }, DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);

		i32 upload_pitch = aligned_forward(width * 4, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
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

		Gfx::Pass* transition_to_readable_pass = device_->graph_.AddSubsequentPass(Gfx::PassAttachments{}.Attach({ .resource = *font_texture_.resource_ }, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		encoder.SetPass(transition_to_readable_pass);

		encoder.Submit();
		device_->GetWaitable().Wait();
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
			Gfx::ShaderBlob* vs_bytecode = Gfx::CompileShader(vs_blob, nullptr, L"vs", L"main", L"vs_6_0");
			pso_desc.VS = { vs_bytecode->GetBufferPointer(), vs_bytecode->GetBufferSize() };

			vs_blob->Release();
			vs_bytecode->Release();
		}

		// Create the input layout
		static D3D12_INPUT_ELEMENT_DESC local_layout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (UINT)IM_OFFSETOF(ImDrawVert, pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (UINT)IM_OFFSETOF(ImDrawVert, uv),  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (UINT)IM_OFFSETOF(ImDrawVert, col), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		pso_desc.InputLayout = { local_layout, 3 };

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
			Gfx::ShaderBlob* ps_bytecode = Gfx::CompileShader(ps_blob, nullptr, L"ps", L"main", L"ps_6_0");
			pso_desc.PS = { ps_bytecode->GetBufferPointer(), ps_bytecode->GetBufferSize() };

			ps_blob->Release();
			ps_bytecode->Release();
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
	}

	void RenderDrawData(ImDrawData* draw_data, Gfx::Encoder* encoder) {
		if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
			return;

		ViewportData* render_data = (ViewportData*)draw_data->OwnerViewport->RendererUserData;

		// todo: update vb and ib
		draw_data->TotalVtxCount;
		draw_data->TotalIdxCount;

		for (i32 n = 0, N = draw_data->CmdListsCount; n < N; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
			cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);
		}

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

		int global_vtx_offset = 0;
		int global_idx_offset = 0;
		ImVec2 clip_off = draw_data->DisplayPos;
		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback != nullptr)
				{
					if (pcmd->UserCallback == ImDrawCallback_ResetRenderState) {
						//ImGui_ImplDX12_SetupRenderState(draw_data, ctx, fr);
						assert(0);
					}
					else {
						pcmd->UserCallback(cmd_list, pcmd);
					}
				}
				else
				{
					// Apply Scissor, Bind texture, Draw
					const D3D12_RECT r = { (LONG)(pcmd->ClipRect.x - clip_off.x), (LONG)(pcmd->ClipRect.y - clip_off.y), (LONG)(pcmd->ClipRect.z - clip_off.x), (LONG)(pcmd->ClipRect.w - clip_off.y) };
					//ctx->SetGraphicsRootDescriptorTable(1, *(D3D12_GPU_DESCRIPTOR_HANDLE*)&pcmd->TextureId);
					encoder->GetCmdList()->RSSetScissorRects(1, &r);
					//ctx->DrawIndexedInstanced(pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
				}
			}
			global_idx_offset += cmd_list->IdxBuffer.Size;
			global_vtx_offset += cmd_list->VtxBuffer.Size;
		}
	}
};

int main(int argc, char** argv)
{
	Gfx::Device device;

	InitImgui();
	ImGuiIO& io = ImGui::GetIO();

	ImGuiRenderer imgui_renderer;
	imgui_renderer.Init(&device);

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
	compiled_shader->Release();

	Gfx::Pipeline cs = device.CreateComputePipeline(CS);

	// TODO: recreate resources based on window size
	Gfx::Resource random_access_texture = device.CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, window->resolution_, DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	i32 current_backbuffer_index = 0;

	int frames_ctr = 3;

	Array<Gfx::Waitable> frame_waitables;

	window->RunLoop([
		&window,
			&device,
			&current_backbuffer_index,
			&window_swapchain,
			&random_access_texture,
			&cs,
			backbuffers_num,
			&frames_ctr,
			&frame_waitables,
			&imgui_renderer
	]() {
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			ImGui::Text("Hello, world!");

			ID3D12Resource* current_backbuffer = *window_swapchain->backbuffers_[current_backbuffer_index];

			Gfx::Pass* shader_execution_pass = device.graph_.AddSubsequentPass(Gfx::PassAttachments{}
				.Attach({ .resource = *random_access_texture.resource_ }, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
			);

			Gfx::Pass* copy_to_backbuffer_pass = device.graph_.AddSubsequentPass(Gfx::PassAttachments{}
				.Attach({ .resource = *random_access_texture.resource_ }, D3D12_RESOURCE_STATE_COPY_SOURCE)
				.Attach({ .resource = current_backbuffer }, D3D12_RESOURCE_STATE_COPY_DEST)
			);

			Gfx::Pass* present_pass = device.graph_.AddSubsequentPass(Gfx::PassAttachments{}
				.Attach({ .resource = current_backbuffer }, D3D12_RESOURCE_STATE_PRESENT)
			);

			Gfx::Encoder encoder = device.CreateEncoder();

			ImGui::Render();
			imgui_renderer.RenderDrawData(ImGui::GetDrawData(), &encoder);

			encoder.SetPass(shader_execution_pass);
			ID3D12GraphicsCommandList* cmd_list = encoder.GetCmdList();
			cmd_list->SetPipelineState(*cs.pipeline_);
			D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{
				.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
				.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D,
				.Texture2D = {}
			};
			device.device_->CreateUnorderedAccessView(*random_access_texture.resource_, nullptr, &uav_desc, encoder.ReserveComputeSlot(Gfx::DescriptorType::UAV, 0));
			encoder.SetComputeDescriptors();
			cmd_list->Dispatch((window->resolution_.x() + 7) / 8, (window->resolution_.y() + 3) / 4, 1);

			encoder.SetPass(copy_to_backbuffer_pass);
			cmd_list->CopyResource(current_backbuffer, *random_access_texture.resource_);

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
	ImGui::DestroyContext();

	return 0;
}
