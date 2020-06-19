#pragma once

#include "types.h"
#include "shared_headers.h"
#include "com.h"
#include "assertions.h"
#include "array.h"
#include "copy_move.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3d12memoryallocator/D3D12MemAlloc.h>

#define DX12_ENABLE_DEBUG_LAYER 1

struct gfx_module
	: debug_assert::default_handler, // use the default handler
	debug_assert::set_level<-1> // level -1, i.e. all assertions, 0 would mean none, 1 would be level 1, 2 level 2 or lower,...
{};

#define assert_hr(x) { DEBUG_ASSERT(SUCCEEDED(x), gfx_module{}); }
#define verify_hr(x) { HRESULT __hr = (x); DEBUG_ASSERT(SUCCEEDED(__hr), gfx_module{}); }

namespace Os
{
	struct Window;
}

namespace Gfx
{
	struct Device;
	struct Encoder;
	struct Swapchain;
	struct Resource;

	enum class ResourceType
	{
		Buffer,
		Texture2D
	};

	struct Device : private MoveableNonCopyable<Device>
	{
		Com::UniqueComPtr<IDXGIFactory4> dxgi_factory_;
		Com::UniqueComPtr<IDXGIAdapter1> adapter_;
		Com::UniqueComPtr<ID3D12Device> device_;
		Com::UniqueComPtr<ID3D12CommandQueue> cmd_queue_;
		Com::UniqueComPtr<D3D12MA::Allocator> allocator_;

		Com::UniqueComPtr<ID3D12Fence1> fence_;
		i64 fence_value_ = 0;

		Containers::Array<Com::UniqueComPtr<ID3D12CommandAllocator>> cmd_allocators_;
		Containers::Array<Com::UniqueComPtr<ID3D12CommandList>> cmd_lists_;

		Device();
		~Device();

		void AdvanceFence();
		void WaitForCompletion();

		Encoder CreateEncoder();
		Swapchain CreateSwapchain(Os::Window*, i32 num_backbuffers);
		Resource CreateTexture2D(D3D12_HEAP_TYPE heap_type, Vector2i size, DXGI_FORMAT format, i32 miplevels, D3D12_RESOURCE_FLAGS flags);
	};

	struct Encoder : private MoveableNonCopyable<Encoder>
	{
		Device* device_ = nullptr;

		Com::UniqueComPtr<ID3D12CommandAllocator> cmd_allocator_;
		Com::UniqueComPtr<ID3D12CommandList> cmd_list_;

		ID3D12GraphicsCommandList* GetCmdList();

		void Execute();
	};

	struct Swapchain : private MoveableNonCopyable<Swapchain>
	{
		Com::UniqueComPtr<IDXGISwapChain4> swapchain_;
	};

	struct Resource : private MoveableNonCopyable<Resource>
	{
		Resource() = default;

		ResourceType type_;

		Com::UniqueComPtr<ID3D12Resource> resource_;
		Com::UniqueComPtr<D3D12MA::Allocation> allocation_;
	};
}
