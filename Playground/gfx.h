#pragma once

#include "array.h"
#include "hashmap.h"
#include "com.h"
#include "assertions.h"
#include "copy_move.h"
#include "box.h"

#define DX12_ENABLE_DEBUG_LAYER _DEBUG

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
	using namespace Containers;

	struct Device;
	struct Encoder;
	struct Pass;
	struct Pipeline;
	struct Swapchain;
	struct Resource;
	struct Waitable;

	enum class ResourceType
	{
		Buffer,
		Texture1D,
		Texture2D
	};

	struct SubresourceDesc
	{
		ID3D12Resource* resource;
		u32 subresource;

		bool operator == (SubresourceDesc other) const;
	};

	struct Attachment {
		SubresourceDesc subresource;
		D3D12_RESOURCE_STATES state;
	};

	struct PassAttachments {
		Containers::Array<Attachment> attachments_;

		PassAttachments& Attach(SubresourceDesc, D3D12_RESOURCE_STATES);
	};

	// this is a poor man's graph, it doesn't support branching 
	struct TransitionGraph {
		Hashmap<SubresourceDesc, D3D12_RESOURCE_STATES> last_transitioned_state_;

		struct Node {
			Box<Pass> pass;
			Node* edge = nullptr;
		};

		// TODO: trim the history
		// poor man's ownership
		// keep history for split barriers
		i32 max_depth_ = 100;
		Array<Box<Node>> pending_nodes_;
		Hashmap<Node*, i32> node_index_;

		Node* root_ = nullptr;
		Node* tail_ = nullptr;

		void SetState(SubresourceDesc, D3D12_RESOURCE_STATES);
		void Drop(ID3D12Resource*);
		Pass* AddSubsequentPass(PassAttachments attachments);
	};

	struct Waitable {
		ID3D12Fence1* fence_ = nullptr;
		u64 fence_value_ = 0;

		void Wait();
		bool IsDone();
	};

	enum class DescriptorType {
		SRV,
		UAV,
		CBV
	};

	struct DescriptorHeap
	{
		Com::Box<ID3D12DescriptorHeap> heap_;

		i64 max_slots_ = 0;
		i64 next_slot_ = 0;
		i64 used_start_slot_ = 0;

		struct Fence {
			i64 offset; // index of the first descriptor that might still be in use
			Waitable waitable;
		};
		Array<Fence> fences_;

		// important to call periodically to release older descriptors and avoid deadlock
		// clears all previous usages
		void FenceDescriptors(Waitable waitable);
		i64 AllocateTable(i64 len);

		// TODO: DRY...
		i64 current_graphics_srvs_offset_ = 0;
		Bitarray dirty_graphics_srvs_;

		i64 current_graphics_cbvs_offset_ = 0;
		Bitarray dirty_graphics_cbvs_;

		i64 current_compute_srvs_offset_ = 0;
		Bitarray dirty_compute_srvs_;

		i64 current_compute_uavs_offset_ = 0;
		Bitarray dirty_compute_uavs_;

		i64 current_compute_cbvs_offset_ = 0;
		Bitarray dirty_compute_cbvs_;

		i64 increment_ = 0;
		// TODO: this is hardcoded for current shared RootSignature, make this data driven
	};

	struct Device : private Pinned<Device>
	{
		Com::Box<IDXGIFactory4> dxgi_factory_;
		Com::Box<IDXGIAdapter1> adapter_;
		Com::Box<ID3D12Device> device_;
		Com::Box<ID3D12CommandQueue> cmd_queue_;
		Com::Box<D3D12MA::Allocator> allocator_;

		DescriptorHeap descriptor_heap_;
		DescriptorHeap rtvs_descriptor_heap_;

		Com::Box<ID3D12RootSignature> root_signature_;

		Com::Box<ID3D12Fence1> fence_;
		u64 fence_value_ = 0;

		Array<Com::Box<ID3D12CommandAllocator>> cmd_allocators_;
		Array<Com::Box<ID3D12CommandList>> cmd_lists_;

		Array<Box<Swapchain>> swapchains_;

		TransitionGraph graph_;

		Device();
		~Device();

		void AdvanceFence();
		Waitable GetWaitable();

		Encoder CreateEncoder();
		Swapchain* CreateSwapchain(Os::Window*, i32 num_backbuffers);
		Resource CreateBuffer(D3D12_HEAP_TYPE heap_type, i64 size, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initial_state);
		Resource CreateTexture1D(D3D12_HEAP_TYPE heap_type, i64 size, DXGI_FORMAT format, i32 miplevels, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initial_state);
		Resource CreateTexture2D(D3D12_HEAP_TYPE heap_type, Vector2i size, DXGI_FORMAT format, i32 miplevels, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initial_state);
		Pipeline CreateComputePipeline(D3D12_SHADER_BYTECODE);
	};

	struct Encoder : private MoveableNonCopyable<Encoder>
	{
		Device* device_ = nullptr;

		Com::Box<ID3D12CommandAllocator> cmd_allocator_;
		Com::Box<ID3D12CommandList> cmd_list_;

		ID3D12GraphicsCommandList* GetCmdList();

		void SetPass(Pass*);

		D3D12_CPU_DESCRIPTOR_HANDLE ReserveGraphicsSlot(DescriptorType type, i32 slot_index);
		D3D12_CPU_DESCRIPTOR_HANDLE ReserveComputeSlot(DescriptorType type, i32 slot_index);

		// Currently this "forgets" all the previously reserved slots, as I keep only one shader-visible heap 
		// and it can't be used as a copy src. With a second cpu-visible heap that tracks all the descriptors
		// I could restore the state of previously set descriptors and make this more stateless.
		void SetGraphicsDescriptors();
		void SetComputeDescriptors();

		void Submit();
	};

	struct Swapchain : private Pinned<Swapchain>
	{
		Device* device_ = nullptr;
		Os::Window* window_ = nullptr;

		Com::Box<IDXGISwapChain4> swapchain_;
		i32 backbuffers_num_ = 0;

		Array<Com::Box<ID3D12Resource>> backbuffers_;

		void Destroy();
		void Recreate();
	};

	struct Resource : private MoveableNonCopyable<Resource>
	{
		Resource() = default;
		~Resource();

		Resource(Resource&&) = default;
		Resource& operator=(Resource&&) = default;

		ResourceType type_;
		Device* device_ = nullptr;
		D3D12_HEAP_TYPE heap_type_ = static_cast<D3D12_HEAP_TYPE>(0);

		Com::Box<ID3D12Resource> resource_;
		Com::Box<D3D12MA::Allocation> allocation_;
	};

	struct Pipeline : private MoveableNonCopyable<Resource> {
		Com::Box<ID3D12PipelineState> pipeline_;
	};

	struct Pass {
		Containers::Array<Attachment> attachments_;
	};
}

namespace Hash
{
	template<> u64 HashValue(Gfx::SubresourceDesc);
}