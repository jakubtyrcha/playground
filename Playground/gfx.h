#pragma once

#include "array.h"
#include "hashmap.h"
#include "com.h"
#include "assertions.h"
#include "copy_move.h"

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

	template<typename T> using Box = Corrade::Containers::Pointer<T>;

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
		Pass* AddSubsequentPass(PassAttachments attachments);
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


		i64 AllocateTable(i64 len);

		i64 current_compute_uavs_offset_ = 0;
		Bitarray dirty_compute_uavs_;

		i64 increment_ = 0;
		// TODO: this is hardcoded for current shared RootSignature, make this data driven
	};

	struct Device : private MoveableNonCopyable<Device>
	{
		Com::Box<IDXGIFactory4> dxgi_factory_;
		Com::Box<IDXGIAdapter1> adapter_;
		Com::Box<ID3D12Device> device_;
		Com::Box<ID3D12CommandQueue> cmd_queue_;
		Com::Box<D3D12MA::Allocator> allocator_;

		DescriptorHeap descriptor_heap_;

		Com::Box<ID3D12RootSignature> root_signature_;

		Com::Box<ID3D12Fence1> fence_;
		i64 fence_value_ = 0;

		Array<Com::Box<ID3D12CommandAllocator>> cmd_allocators_;
		Array<Com::Box<ID3D12CommandList>> cmd_lists_;

		TransitionGraph graph_;

		Device();
		~Device();

		void AdvanceFence();
		Waitable GetWaitable();

		Encoder CreateEncoder();
		Swapchain CreateSwapchain(Os::Window*, i32 num_backbuffers);
		Resource CreateTexture2D(D3D12_HEAP_TYPE heap_type, Vector2i size, DXGI_FORMAT format, i32 miplevels, D3D12_RESOURCE_FLAGS flags);
		Pipeline CreateComputePipeline(D3D12_SHADER_BYTECODE);
	};

	struct Encoder : private MoveableNonCopyable<Encoder>
	{
		Device* device_ = nullptr;

		Com::Box<ID3D12CommandAllocator> cmd_allocator_;
		Com::Box<ID3D12CommandList> cmd_list_;

		ID3D12GraphicsCommandList* GetCmdList();

		D3D12_CPU_DESCRIPTOR_HANDLE ReserveComputeSlot(DescriptorType type, i32 slot_index);
		void SetPass(Pass*);
		void SetComputeDescriptors();
		void Submit();
	};

	struct Swapchain : private MoveableNonCopyable<Swapchain>
	{
		Com::Box<IDXGISwapChain4> swapchain_;

		Array<Com::Box<ID3D12Resource>> backbuffers_;
	};

	struct Resource : private MoveableNonCopyable<Resource>
	{
		Resource() = default;

		ResourceType type_;

		Com::Box<ID3D12Resource> resource_;
		Com::Box<D3D12MA::Allocation> allocation_;
	};

	struct Pipeline : private MoveableNonCopyable<Resource> {
		Com::Box<ID3D12PipelineState> pipeline_;
	};

	struct Pass {
		Containers::Array<Attachment> attachments_;
	};

	struct Waitable {
		ID3D12Fence1* fence_;
		i64 fence_value_ = 0;

		void Wait();
	};
}

namespace Hash
{
	template<> u64 HashValue(Gfx::SubresourceDesc);
}