#include "gfx.h"
#include "os.h"
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include "assertions.h"

namespace Gfx
{
	IDXGIAdapter1* FindAdapter(IDXGIFactory4* dxgi_factory)
	{
		IDXGIAdapter1* adapter = nullptr;
		int adapter_index = 0;
		bool adapter_found = false;
		while (dxgi_factory->EnumAdapters1(adapter_index, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0)
			{
				HRESULT hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), nullptr);
				if (SUCCEEDED(hr))
				{
					adapter_found = true;
					break;
				}
			}
			adapter->Release();
			adapter_index++;
		}
		assert(adapter_found);

		return adapter;
	}

	Device::Device()
	{
		verify_hr(CreateDXGIFactory1(IID_PPV_ARGS(dxgi_factory_.InitAddress())));

		*adapter_.InitAddress() = FindAdapter(dxgi_factory_.Get());

#ifdef DX12_ENABLE_DEBUG_LAYER
		ID3D12Debug* d3d12_debug = NULL;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&d3d12_debug))))
		{
			d3d12_debug->EnableDebugLayer();
			d3d12_debug->Release();
		}
#endif

		D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_12_1;
		HRESULT hr = D3D12CreateDevice(adapter_.Get(), feature_level, IID_PPV_ARGS(device_.InitAddress()));
		DEBUG_ASSERT(hr == S_OK, gfx_module{});

		D3D12MA::ALLOCATOR_DESC allocator_desc{};
		allocator_desc.pAdapter = *adapter_;
		allocator_desc.pDevice = *device_;
		verify_hr(D3D12MA::CreateAllocator(&allocator_desc, allocator_.InitAddress()));

		D3D12_COMMAND_QUEUE_DESC queue_desc{};
		queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		verify_hr(device_->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(cmd_queue_.InitAddress())));

		verify_hr(device_->CreateFence(fence_value_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence_.InitAddress())));

		/*
		D3D12_FEATURE_DATA_D3D12_OPTIONS options;
		hr = device_->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options));
		DEBUG_ASSERT(hr == S_OK, gfx_module{});

		D3D12_FEATURE_DATA_SHADER_MODEL shader_model;
		shader_model.HighestShaderModel = D3D_SHADER_MODEL_6_4;
		hr = device_->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shader_model, sizeof(shader_model));
		DEBUG_ASSERT(hr == S_OK, gfx_module{});

		D3D12_FEATURE_DATA_D3D12_OPTIONS1 options1;
		hr = device_->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &options1, sizeof(options1));
		DEBUG_ASSERT(hr == S_OK, gfx_module{});

		D3D12_FEATURE_DATA_D3D12_OPTIONS2 options2;
		hr = device_->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS2, &options2, sizeof(options2));
		DEBUG_ASSERT(hr == S_OK, gfx_module{});

		D3D12_FEATURE_DATA_D3D12_OPTIONS3 options3;
		hr = device_->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3, &options3, sizeof(options3));
		DEBUG_ASSERT(hr == S_OK, gfx_module{});*/

		/*D3D12_FEATURE_DATA_D3D12_OPTIONS6 options6;
		hr = device_->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &options6, sizeof(options6));
		DEBUG_ASSERT(hr == S_OK, gfx_module{});*/
	}

	Device::~Device()
	{
#ifdef DX12_ENABLE_DEBUG_LAYER
		IDXGIDebug1* dxgi_debug = NULL;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgi_debug))))
		{
			dxgi_debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
			dxgi_debug->Release();
		}
#endif
	}

	void Device::AdvanceFence()
	{
		verify_hr(cmd_queue_->Signal(*fence_, ++fence_value_));
	}

	void Device::WaitForCompletion()
	{
		HANDLE event = CreateEvent(nullptr, false, false, nullptr);
		verify_hr(fence_->SetEventOnCompletion(fence_value_, event));
		verify(WaitForSingleObject(event, INFINITE) == WAIT_OBJECT_0);
		CloseHandle(event);
	}

	Encoder Device::CreateEncoder()
	{
		Encoder result{};
		result.device_ = this;

		if (cmd_allocators_.Size())
		{
			result.cmd_allocator_ = cmd_allocators_.PopBack();
		}
		else
		{
			verify_hr(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(result.cmd_allocator_.InitAddress())));
			verify_hr(result.cmd_allocator_->Reset());
		}

		if (cmd_lists_.Size())
		{
			result.cmd_list_ = cmd_lists_.PopBack();
		}
		else
		{
			verify_hr(device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, *result.cmd_allocator_, nullptr, IID_PPV_ARGS(result.cmd_list_.InitAddress())));
		}

		return result;
	}

	Swapchain Device::CreateSwapchain(Os::Window* window, i32 num_backbuffers)
	{
		Swapchain result{};

		IDXGISwapChain1* swapchain1 = nullptr;
		DXGI_SWAP_CHAIN_DESC1 swapchain_desc{};
		swapchain_desc.BufferCount = num_backbuffers;
		swapchain_desc.Width = window->resolution_.x();
		swapchain_desc.Height = window->resolution_.y();
		swapchain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapchain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
		swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchain_desc.SampleDesc.Count = 1;
		swapchain_desc.SampleDesc.Quality = 0;
		swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapchain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapchain_desc.Scaling = DXGI_SCALING_STRETCH;
		swapchain_desc.Stereo = FALSE;
		dxgi_factory_->CreateSwapChainForHwnd(*cmd_queue_, window->hwnd_, &swapchain_desc, nullptr, nullptr, &swapchain1);

		verify_hr(swapchain1->QueryInterface<IDXGISwapChain4>(result.swapchain_.InitAddress()));
		swapchain1->Release();

		result.swapchain_->SetMaximumFrameLatency(num_backbuffers);

		return result;
	}

	Resource Device::CreateTexture2D(D3D12_HEAP_TYPE heap_type, Vector2i size, DXGI_FORMAT format, i32 miplevels, D3D12_RESOURCE_FLAGS flags)
	{
		Resource result{};
		result.type_ = ResourceType::Texture2D;

		D3D12_RESOURCE_DESC resource_desc = {};
		resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resource_desc.Alignment = 0;
		resource_desc.Width = size.x();
		resource_desc.Height = size.y();
		resource_desc.DepthOrArraySize = 1;
		resource_desc.MipLevels = miplevels;
		resource_desc.Format = format;
		resource_desc.SampleDesc.Count = 1;
		resource_desc.SampleDesc.Quality = 0;
		resource_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resource_desc.Flags = flags;

		D3D12MA::ALLOCATION_DESC allocation_desc = {};
		allocation_desc.HeapType = heap_type;

		verify_hr(allocator_->CreateResource(
			&allocation_desc,
			&resource_desc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			NULL,
			result.allocation_.InitAddress(),
			IID_PPV_ARGS(result.resource_.InitAddress())));

		return result;
	}

	ID3D12GraphicsCommandList* Encoder::GetCmdList()
	{
		return static_cast<ID3D12GraphicsCommandList*>(*cmd_list_);
	}

	void Encoder::Execute()
	{
		verify_hr(GetCmdList()->Close());
		ID3D12CommandList* cmd_list = GetCmdList();
		device_->cmd_queue_->ExecuteCommandLists(1, &cmd_list);
		device_->cmd_allocators_.PushBackRvalueRef(std::move(cmd_allocator_));
		device_->cmd_lists_.PushBackRvalueRef(std::move(cmd_list_));
		device_->AdvanceFence();

		device_ = nullptr;
	}
}