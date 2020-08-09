#pragma once

#include "Types.h"
#include "array.h"
#include "assertions.h"
#include "box.h"
#include "com.h"
#include "hashmap.h"
#include "shader.h"
#include "FreeList.h"
#include <magnum/CorradeOptional.h>

struct gfx_module
    : debug_assert::default_handler, // use the default handler
      debug_assert::set_level<-1> // level -1, i.e. all assertions, 0 would mean none, 1 would be level 1, 2 level 2 or lower,...
{
};

#define assert_hr(x)                               \
    {                                              \
        DEBUG_ASSERT(SUCCEEDED(x), gfx_module {}); \
    }
#define verify_hr(x)                                  \
    {                                                 \
        HRESULT __hr = (x);                           \
        DEBUG_ASSERT(SUCCEEDED(__hr), gfx_module {}); \
    }

namespace Playground {

namespace Os {
struct Window;
}

namespace Gfx {

struct Device;
struct Encoder;
struct Pass;
struct Pipeline;
struct Swapchain;
struct Resource;
struct Waitable;

enum class ResourceType {
    Buffer,
    Texture1D,
    Texture2D
};

struct SubresourceDesc {
    ID3D12Resource* resource;
    i32 subresource;

    bool operator==(SubresourceDesc other) const;
};

enum DescriptorHandleType {
    Other, 
    Rtv,
    Dsv,
    Srv
};

struct DescriptorHandle {
    u64 handle;
};

enum class Lifetime {
    Frame,
    Manual
};

struct Attachment {
    SubresourceDesc subresource;
    D3D12_RESOURCE_STATES state;
};

struct PassAttachments {
    Array<Attachment> attachments_;

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
    Device* device_ = nullptr;

    i32 handle_ = 0;
    i32 generation_ = 0;

    void Wait();
    bool IsDone();
};

enum class DescriptorType {
    SRV,
    UAV,
    CBV
};

struct DescriptorHeap {
    Com::Box<ID3D12DescriptorHeap> heap_;

    i64 max_slots_ = 0;
    i64 next_slot_ = 0;
    i64 used_start_slot_ = 0;

    struct Fence {
        i64 offset; // index of the first descriptor that might still be in use
        Waitable waitable;
    };
    Array<Fence> fences_;

    void Init(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_DESC desc);

    // important to call periodically to release older descriptors and avoid deadlock
    // clears all previous usages
    void FenceDescriptors(Waitable waitable);
    i64 AllocateTable(i64 len);

    struct Table {
        i64 offset = 0;
        Bitarray dirty;

        void Resize(i32 size);
    };

    D3D12_CPU_DESCRIPTOR_HANDLE ReserveTableSlot(Table& table, i32 slot_index);
    template <typename F, typename F1>
    void SetRootTable(Table& table, F fill_null_slot, F1 set_root_param);

    struct TablesSet {
        Table srv;
        Table uav;
        Table cbv;
    };

    TablesSet graphics_tables;
    TablesSet compute_tables;

    i64 increment_ = 0;
    // TODO: this is hardcoded for the current shared RootSignature, make this data driven
};

struct Device : private Pinned<Device> {
    struct ReleaseSet {
        Waitable waitable;
        Array<Resource> resources;
        Array<DescriptorHandle> handles;
    };

    Array<ReleaseSet> release_sets_queue_;

    Com::Box<IDXGIFactory4> dxgi_factory_;
    Com::Box<IDXGIAdapter1> adapter_;
    Com::Box<ID3D12Device> device_;
    Com::Box<ID3D12CommandQueue> cmd_queue_;
    D3D12MA::Allocator* allocator_ = nullptr;

    DescriptorHeap frame_descriptor_heap_;

    FreeList manual_descriptor_heap_freelist_;
    DescriptorHeap manual_descriptor_heap_;

    FreeList manual_rtv_descriptor_heap_freelist_;
    DescriptorHeap manual_rtv_descriptor_heap_;

    FreeList manual_dsv_descriptor_heap_freelist_;
    DescriptorHeap manual_dsv_descriptor_heap_;

    DescriptorHeap descriptor_heap_;
    DescriptorHeap rtvs_descriptor_heap_;
    DescriptorHeap dsvs_descriptor_heap_;

    Com::Box<ID3D12RootSignature> root_signature_;

    Com::Box<ID3D12Fence1> fence_;
    u64 fence_value_ = 0;

    struct WaitableSlot {
        Optional<u64> value;
        i32 generation = 0;
        bool pending = false;
    };
    Array<WaitableSlot> waitables_pool_;
    // TODO: queue candidate
    Array<Waitable> waitables_pending_;

    struct PendingCommandAllocator {
        Com::Box<ID3D12CommandAllocator> allocator;
        Waitable waitable;
    };

    Array<PendingCommandAllocator> cmd_allocators_pending_;
    Array<Com::Box<ID3D12CommandAllocator>> cmd_allocators_;
    Array<Com::Box<ID3D12CommandList>> cmd_lists_;

    Array<Box<Swapchain>> swapchains_;

    TransitionGraph graph_;

    Device();
    ~Device();

    void _Submit(Encoder&& encoder);

    void AdvanceFence();
    Waitable GetWaitable();

    Waitable ReserveWaitable();
    void TriggerWaitable(Waitable, u64);
    void Wait(Waitable);
    bool IsDone(Waitable);

    void ReleaseWhenCurrentFrameIsDone(Resource &&);
    void ReleaseWhenCurrentFrameIsDone(DescriptorHandle);
    void Release(DescriptorHandle, DescriptorHandleType);

    // has to be called from time to time (once per frame) to free older waitables, descriptors etc
    void RecycleResources();

    Encoder CreateEncoder();
    Swapchain* CreateSwapchain(Os::Window*, i32 num_backbuffers);

    DescriptorHandle CreateDescriptor(Resource *, D3D12_SHADER_RESOURCE_VIEW_DESC const& desc, Lifetime lifetime);
    DescriptorHandle CreateDescriptor(D3D12_CONSTANT_BUFFER_VIEW_DESC const& desc, Lifetime lifetime);
    DescriptorHandle CreateDescriptor(Resource *, D3D12_RENDER_TARGET_VIEW_DESC const& desc, Lifetime lifetime);
    DescriptorHandle CreateDescriptor(Resource *, D3D12_DEPTH_STENCIL_VIEW_DESC const& desc, Lifetime lifetime);
    D3D12_CPU_DESCRIPTOR_HANDLE _GetSrcHandle(DescriptorHandle handle);

    Resource CreateBuffer(D3D12_HEAP_TYPE heap_type, i64 size, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initial_state);
    Resource CreateTexture1D(D3D12_HEAP_TYPE heap_type, i64 size, DXGI_FORMAT format, i32 miplevels, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initial_state);
    Resource CreateTexture2D(D3D12_HEAP_TYPE heap_type, Vector2i size, DXGI_FORMAT format, i32 miplevels, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initial_state);

    static void ShutdownAllocator();
};

struct Encoder : private MoveableNonCopyable<Encoder> {
    Device* device_ = nullptr;

    Com::Box<ID3D12CommandAllocator> cmd_allocator_;
    Com::Box<ID3D12CommandList> cmd_list_;
    Array<Waitable> waitables_to_trigger_;

    ID3D12GraphicsCommandList* GetCmdList();

    void SetPass(Pass*);

    D3D12_CPU_DESCRIPTOR_HANDLE ReserveGraphicsSlot(DescriptorType type, i32 slot_index);
    D3D12_CPU_DESCRIPTOR_HANDLE ReserveComputeSlot(DescriptorType type, i32 slot_index);

    void SetGraphicsDescriptor(DescriptorType type, i32 slot_index, DescriptorHandle handle);

    // Currently this "forgets" all the previously reserved slots, as I keep only one shader-visible heap
    // and it can't be used as a copy src. With a second cpu-visible heap that tracks all the descriptors
    // I could restore the state of previously set descriptors and make this more stateless.
    void SetGraphicsDescriptors();
    void SetComputeDescriptors();

    Waitable GetWaitable();

    void Submit();
};

struct RenderTargetDesc {
    Resource* resource;
    DescriptorHandle rtv;
    DXGI_FORMAT fmt;
};

struct Swapchain : private Pinned<Swapchain> {
    Device* device_ = nullptr;
    Os::Window* window_ = nullptr;

    Com::Box<IDXGISwapChain4> swapchain_;
    i32 backbuffers_num_ = 0;

    Array<Resource> backbuffers_;
    Array<RenderTargetDesc> backbuffer_descs_;

    void Destroy();
    void Recreate();

    RenderTargetDesc GetCurrentBackbufferAsRenderTarget();
};

struct Resource : private MoveableNonCopyable<Resource> {
    Resource() = default;
    ~Resource();

    Resource(Resource&&) = default;
    Resource& operator=(Resource&&) = default;

    ResourceType type_;
    Device* device_ = nullptr;
    D3D12_HEAP_TYPE heap_type_ = static_cast<D3D12_HEAP_TYPE>(0);
    i32 subresources_num_ = 0;

    Com::Box<ID3D12Resource> resource_;
    Com::Box<D3D12MA::Allocation> allocation_;

    static Resource From(Device * device, ID3D12Resource * ptr);
};

struct Pipeline : private MoveableNonCopyable<Resource> {
    Com::Box<ID3D12PipelineState> pipeline_;

    static Optional<Box<Pipeline>> From(Device* device, D3D12_GRAPHICS_PIPELINE_STATE_DESC const& desc);
};

struct Pass {
    Array<Attachment> attachments_;
};

// utils
D3D12_GRAPHICS_PIPELINE_STATE_DESC GetDefaultPipelineStateDesc(Device* device);
void UpdateTexture2DSubresource(Device* device, Resource* resource, i32 subresource, Vector2i resource_size, DXGI_FORMAT fmt, const void* src, i32 src_pitch, i32 rows);
void UpdateBuffer(Device* device, Resource* resource, const void* src, i32 bytes, Optional<D3D12_RESOURCE_STATES> post_transition);

DescriptorHandle MakeFrameCbv(Device* device, Resource* resource, i32 bytes);

template <typename T>
DescriptorHandle MakeFrameCbv(Device* device, Resource* resource, i32 data_count)
{
    return MakeFrameCbv(device, resource, sizeof(T) * data_count);
}

DescriptorHandle MakeFrameCbvFromMemory(Device* device, void const * src, i32 bytes);

template <typename T>
DescriptorHandle MakeFrameCbvFromMemory(Device* device, const T& ptr)
{
    return MakeFrameCbvFromMemory(device, &ptr, As<i32>(sizeof(T)));
}
}

namespace Hash {
template <>
u64 HashValue(Gfx::SubresourceDesc);
}

}