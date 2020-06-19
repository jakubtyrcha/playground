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

	D3D12MA::Allocator* gfx_allocator = nullptr;
	D3D12MA::ALLOCATOR_DESC allocator_desc{};
	allocator_desc.pAdapter = device.adapter_.Get();
	allocator_desc.pDevice = device.device_.Get();
	verify_hr(D3D12MA::CreateAllocator(&allocator_desc, &gfx_allocator));

	D3D12_RESOURCE_DESC resource_desc = {};
	resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resource_desc.Alignment = 0;
	resource_desc.Width = 1024;
	resource_desc.Height = 1024;
	resource_desc.DepthOrArraySize = 1;
	resource_desc.MipLevels = 1;
	resource_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resource_desc.SampleDesc.Count = 1;
	resource_desc.SampleDesc.Quality = 0;
	resource_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resource_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	D3D12MA::ALLOCATION_DESC allocation_desc = {};
	allocation_desc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

	ID3D12Resource* texture = nullptr;
	D3D12MA::Allocation* allocation = nullptr;
	verify_hr(gfx_allocator->CreateResource(
		&allocation_desc,
		&resource_desc,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		NULL,
		&allocation,
		IID_PPV_ARGS(&texture)));

	ID3D12CommandQueue* cmd_queue = nullptr;

	D3D12_COMMAND_QUEUE_DESC queue_desc{};
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	verify_hr(device.device_->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&cmd_queue)));

	ID3D12CommandAllocator* cmd_allocator = nullptr;
	verify_hr(device.device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmd_allocator)));
	cmd_allocator->Reset();

	ID3D12GraphicsCommandList* cmd_list = nullptr;
	verify_hr(device.device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmd_allocator, pso, IID_PPV_ARGS(&cmd_list)));

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
	device.device_->CreateUnorderedAccessView(texture, nullptr, &uav_desc, descriptor_heap->GetCPUDescriptorHandleForHeapStart());

	cmd_list->SetComputeRootSignature(root_signature);
	cmd_list->SetDescriptorHeaps(1, &descriptor_heap);
	cmd_list->SetComputeRootDescriptorTable(0, descriptor_heap->GetGPUDescriptorHandleForHeapStart());
	cmd_list->Dispatch(1024 / 8, 1024 / 4, 1);
	verify_hr(cmd_list->Close());

	ID3D12Fence1* fence = nullptr;
	verify_hr(device.device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

	cmd_queue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&cmd_list));
	cmd_queue->Signal(fence, 1);

	HANDLE event = CreateEvent(nullptr, false, false, nullptr);

	verify_hr(fence->SetEventOnCompletion(1, event));

	verify(WaitForSingleObject(event, INFINITE) == WAIT_OBJECT_0);

	Os::Window window{ Vector2i{1920, 1080}, &WndProc };

	IDXGISwapChain1* swapchain1 = nullptr;
	i32 num_backbuffers = 3;
	DXGI_SWAP_CHAIN_DESC1 swapchain_desc{};
	swapchain_desc.BufferCount = num_backbuffers;
	swapchain_desc.Width = 0;
	swapchain_desc.Height = 0;
	swapchain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
	swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchain_desc.SampleDesc.Count = 1;
	swapchain_desc.SampleDesc.Quality = 0;
	swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchain_desc.Scaling = DXGI_SCALING_STRETCH;
	swapchain_desc.Stereo = FALSE;
	device.dxgi_factory_->CreateSwapChainForHwnd(cmd_queue, window.hwnd_, &swapchain_desc, nullptr, nullptr, &swapchain1);

	IDXGISwapChain4* swapchain = nullptr;
	verify_hr(swapchain1->QueryInterface<IDXGISwapChain4>(&swapchain));
	swapchain1->Release();

	swapchain->SetMaximumFrameLatency(num_backbuffers);

	window.RunLoop([&swapchain1]() {});

	swapchain->Release();
	descriptor_heap->Release();
	fence->Release();
	pso->Release();
	root_signature->Release();
	texture->Release();
	allocation->Release();
	cmd_list->Release();
	cmd_allocator->Release();
	cmd_queue->Release();
	gfx_allocator->Release();

	CloseHandle(event);

	return 0;
}
