#include "gfx.h"
#include <fmt/format.h>
#include "array.h"
#include "assertions.h"
#include "os.h"
#include "files.h"
//#include <dxcapi.h>

//#include <magnum/CorradeOptional.h>
//template<typename T>
//using Optional = Corrade::Containers::Optional<T>;

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

	D3D12_VERSIONED_ROOT_SIGNATURE_DESC root_signature_desc{};
	root_signature_desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
	root_signature_desc.Desc_1_1.NumParameters = 1;
	D3D12_ROOT_PARAMETER1 params[1] = {};
	params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	D3D12_DESCRIPTOR_RANGE1 ranges[1] = {};
	ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	ranges[0].NumDescriptors = 1;
	ranges[0].OffsetInDescriptorsFromTableStart = 0;
	params[0].DescriptorTable.NumDescriptorRanges = 1;
	params[0].DescriptorTable.pDescriptorRanges = ranges;
	params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	root_signature_desc.Desc_1_1.pParameters = params;

	ID3DBlob* serialized_root_sig = nullptr;
	ID3DBlob* root_sig_errors = nullptr;
	HRESULT hr = D3D12SerializeVersionedRootSignature(&root_signature_desc, &serialized_root_sig, &root_sig_errors);
	if (root_sig_errors)
	{
		fmt::print("RootSignature serialisation errors: {}", static_cast<const char*>(root_sig_errors->GetBufferPointer()));
	}
	assert_hr(hr);

	ID3D12RootSignature* root_signature = nullptr;
	verify_hr(device.device_->CreateRootSignature(0, serialized_root_sig->GetBufferPointer(), serialized_root_sig->GetBufferSize(), IID_PPV_ARGS(&root_signature)));

	Array<char> shader = IO::GetFileContent(L"../data/shader.dxil");

	D3D12_COMPUTE_PIPELINE_STATE_DESC pipeline_desc{};
	pipeline_desc.pRootSignature = root_signature;
	pipeline_desc.CS.pShaderBytecode = shader.Data();
	pipeline_desc.CS.BytecodeLength = shader.Size();

	ID3D12PipelineState* pso = nullptr;
	verify_hr(device.device_->CreateComputePipelineState(&pipeline_desc, IID_PPV_ARGS(&pso)));

	Gfx::Resource random_access_texture = device.CreateTexture2D(D3D12_HEAP_TYPE_DEFAULT, { 1024, 1024 }, DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	Gfx::Encoder encoder = device.CreateEncoder();

	ID3D12DescriptorHeap* descriptor_heap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
	heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heap_desc.NumDescriptors = 128;
	heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	verify_hr(device.device_->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&descriptor_heap)));

	D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
	uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uav_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uav_desc.Texture2D.MipSlice = 0;
	uav_desc.Texture2D.PlaneSlice = 0;
	device.device_->CreateUnorderedAccessView(*random_access_texture.resource_, nullptr, &uav_desc, descriptor_heap->GetCPUDescriptorHandleForHeapStart());

	ID3D12GraphicsCommandList* cmd_list = encoder.GetCmdList();
	cmd_list->SetComputeRootSignature(root_signature);
	cmd_list->SetDescriptorHeaps(1, &descriptor_heap);
	cmd_list->SetPipelineState(pso);
	cmd_list->SetComputeRootDescriptorTable(0, descriptor_heap->GetGPUDescriptorHandleForHeapStart());
	cmd_list->Dispatch(1024 / 8, 1024 / 4, 1);

	encoder.Execute();

	i32 num_backbuffers = 3;
	Os::Window window{ Vector2i{1920, 1080}, &WndProc };
	Gfx::Swapchain window_swapchain = device.CreateSwapchain(&window, num_backbuffers);

	window.RunLoop([]() {});

	device.WaitForCompletion();

	descriptor_heap->Release();
	pso->Release();
	root_signature->Release();

	return 0;
}
