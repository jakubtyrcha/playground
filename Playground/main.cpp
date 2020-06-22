#include "pch.h"

#include "gfx.h"
#include <fmt/format.h>
#include "array.h"
#include "assertions.h"
#include "os.h"
#include "files.h"

#include "shader.h"
#include <dxcapi.h>

using namespace Containers;
using namespace IO;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	/*if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;*/

	switch (msg)
	{
	case WM_SIZE:
		if (/*g_pd3dDevice != NULL &&*/ wParam != SIZE_MINIMIZED)
		{
			/*WaitForLastSubmittedFrame();
			ImGui_ImplDX12_InvalidateDeviceObjects();
			CleanupRenderTarget();
			ResizeSwapChain(hWnd, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
			CreateRenderTarget();
			ImGui_ImplDX12_CreateDeviceObjects();*/
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

int main(int argc, char** argv)
{
	Gfx::Device device;

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

	Gfx::Resource random_access_texture = device.CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, { 1920, 1080 }, DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	{
		Gfx::Encoder encoder = device.CreateEncoder();
		ID3D12GraphicsCommandList* cmd_list = encoder.GetCmdList();
		cmd_list->SetPipelineState(*cs.pipeline_);
		D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D
		};

		device.device_->CreateUnorderedAccessView(*random_access_texture.resource_, nullptr, &uav_desc, encoder.ReserveComputeSlot(Gfx::DescriptorType::UAV, 0));
		encoder.SetComputeDescriptors();
		cmd_list->Dispatch(1920 / 8, 1080 / 4, 1);
		encoder.Submit();
	}

	i32 num_backbuffers = 3;
	Os::Window window{ Vector2i{1920, 1080}, &WndProc };
	Gfx::Swapchain window_swapchain = device.CreateSwapchain(&window, num_backbuffers);

	i32 current_backbuffer_index = 0;

	device.graph_.SetState({ .resource = *random_access_texture.resource_ }, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	for (i32 i = 0; i < num_backbuffers; i++) {
		device.graph_.SetState({ .resource = *window_swapchain.backbuffers_[i] }, D3D12_RESOURCE_STATE_PRESENT);
	}

	int frames_ctr = 3;

	Array<Gfx::Waitable> frame_waitables;

	window.RunLoop([&window, &device, &current_backbuffer_index, &window_swapchain, &random_access_texture, &cs, num_backbuffers, &frames_ctr, &frame_waitables]() {

		ID3D12Resource* current_backbuffer = *window_swapchain.backbuffers_[current_backbuffer_index];

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
		cmd_list->Dispatch(1920 / 8, 1080 / 4, 1);

		encoder.SetPass(copy_to_backbuffer_pass);
		cmd_list->CopyResource(current_backbuffer, *random_access_texture.resource_);

		encoder.SetPass(present_pass);

		// we should wait for the presenting being done before we access that backbuffer again?
		if (frame_waitables.Size() == num_backbuffers) {
			frame_waitables.RemoveAt(0).Wait();
		}

		encoder.Submit();

		verify_hr(window_swapchain.swapchain_->Present(1, 0));
		device.AdvanceFence();

		frame_waitables.PushBack(device.GetWaitable());

		current_backbuffer_index = (current_backbuffer_index + 1) % num_backbuffers;

		});

	device.GetWaitable().Wait();

	return 0;
}
