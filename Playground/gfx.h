#pragma once

#include "shared_headers.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3d12memoryallocator/D3D12MemAlloc.h>
#include "com.h"
#include "assertions.h"

#define DX12_ENABLE_DEBUG_LAYER 1

struct gfx_module
	: debug_assert::default_handler, // use the default handler
	debug_assert::set_level<-1> // level -1, i.e. all assertions, 0 would mean none, 1 would be level 1, 2 level 2 or lower,...
{};

#define assert_hr(x) { DEBUG_ASSERT(SUCCEEDED(x), gfx_module{}); }
#define verify_hr(x) { HRESULT __hr = (x); DEBUG_ASSERT(SUCCEEDED(__hr), gfx_module{}); }

namespace Gfx
{
	struct Device
	{
		Com::UniqueComPtr<IDXGIFactory4> dxgi_factory_;
		Com::UniqueComPtr<IDXGIAdapter1> adapter_;
		Com::UniqueComPtr<ID3D12Device> device_;

		Device();
		~Device();
	};

	enum class ResourceType
	{
		Buffer,
		Texture2D
	};

	struct Resource
	{
		ResourceType type_;
	};

	struct Swapchain
	{

	};
}
