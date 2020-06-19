#include "gfx.h"
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
		verify_hr(CreateDXGIFactory1(IID_PPV_ARGS(dxgi_factory_.PtrAddress())));

		*adapter_.PtrAddress() = FindAdapter(dxgi_factory_.Get());

#ifdef DX12_ENABLE_DEBUG_LAYER
		ID3D12Debug* d3d12_debug = NULL;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&d3d12_debug))))
		{
			d3d12_debug->EnableDebugLayer();
			d3d12_debug->Release();
		}
#endif

		D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_12_1;
		HRESULT hr = D3D12CreateDevice(adapter_.Get(), feature_level, IID_PPV_ARGS(device_.PtrAddress()));
		DEBUG_ASSERT(hr == S_OK, gfx_module{});

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
		DEBUG_ASSERT(hr == S_OK, gfx_module{});

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
}