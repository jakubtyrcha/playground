#include "gfx.h"
#include "os.h"
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include "assertions.h"
#include <fmt/format.h>

namespace Gfx
{
	bool SubresourceDesc::operator==(SubresourceDesc other) const
	{
		return resource == other.resource && subresource == other.subresource;
	}

	PassAttachments& PassAttachments::Attach(SubresourceDesc subresource, D3D12_RESOURCE_STATES state) {
		attachments_.PushBack({ .subresource = subresource, .state = state });
		return *this;
	}

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
		verify_hr(D3D12CreateDevice(adapter_.Get(), feature_level, IID_PPV_ARGS(device_.InitAddress())));

		D3D12MA::ALLOCATOR_DESC allocator_desc{};
		allocator_desc.pAdapter = *adapter_;
		allocator_desc.pDevice = *device_;
		verify_hr(D3D12MA::CreateAllocator(&allocator_desc, allocator_.InitAddress()));

		D3D12_COMMAND_QUEUE_DESC queue_desc{};
		queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		verify_hr(device_->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(cmd_queue_.InitAddress())));

		verify_hr(device_->CreateFence(fence_value_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence_.InitAddress())));

		D3D12_FEATURE_DATA_D3D12_OPTIONS options;
		verify_hr(device_->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options)));

		/*
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

		D3D12_VERSIONED_ROOT_SIGNATURE_DESC root_signature_desc{};

		root_signature_desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;

		D3D12_ROOT_PARAMETER1 params[3] = {};

		params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		D3D12_DESCRIPTOR_RANGE1 ranges_param0[1] = {};
		ranges_param0[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		ranges_param0[0].NumDescriptors = 8;
		ranges_param0[0].OffsetInDescriptorsFromTableStart = 0; //
		params[0].DescriptorTable.NumDescriptorRanges = _countof(ranges_param0);
		params[0].DescriptorTable.pDescriptorRanges = ranges_param0;
		params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		D3D12_DESCRIPTOR_RANGE1 ranges_param1[1] = {};
		ranges_param1[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		ranges_param1[0].NumDescriptors = 8;
		ranges_param1[0].OffsetInDescriptorsFromTableStart = 0; //
		params[1].DescriptorTable.NumDescriptorRanges = _countof(ranges_param1);
		params[1].DescriptorTable.pDescriptorRanges = ranges_param1;
		params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		D3D12_DESCRIPTOR_RANGE1 ranges_param2[1] = {};
		ranges_param2[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		ranges_param2[0].NumDescriptors = 8;
		ranges_param2[0].OffsetInDescriptorsFromTableStart = 0; //
		params[2].DescriptorTable.NumDescriptorRanges = _countof(ranges_param2);
		params[2].DescriptorTable.pDescriptorRanges = ranges_param2;
		params[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		root_signature_desc.Desc_1_1.NumParameters = _countof(params);
		root_signature_desc.Desc_1_1.pParameters = params;

		ID3DBlob* serialized_root_sig = nullptr;
		ID3DBlob* root_sig_errors = nullptr;
		HRESULT hr = D3D12SerializeVersionedRootSignature(&root_signature_desc, &serialized_root_sig, &root_sig_errors);
		if (root_sig_errors)
		{
			fmt::print("RootSignature serialisation errors: {}", static_cast<const char*>(root_sig_errors->GetBufferPointer()));
		}
		assert_hr(hr);

		verify_hr(device_->CreateRootSignature(0, serialized_root_sig->GetBufferPointer(), serialized_root_sig->GetBufferSize(), IID_PPV_ARGS(root_signature_.InitAddress())));

		D3D12_DESCRIPTOR_HEAP_DESC heap_desc{
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			.NumDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
		};
		verify_hr(device_->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(descriptor_heap_.heap_.InitAddress())));

		descriptor_heap_.increment_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		descriptor_heap_.max_slots_ = heap_desc.NumDescriptors;
		descriptor_heap_.dirty_compute_uavs_.Resize(8);
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

	Waitable Device::GetWaitable() {
		return { .fence_ = fence_.Get(), .fence_value_ = fence_value_ };
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
			verify_hr(result.GetCmdList()->Reset(*result.cmd_allocator_, nullptr));
		}
		else
		{
			verify_hr(device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, *result.cmd_allocator_, nullptr, IID_PPV_ARGS(result.cmd_list_.InitAddress())));
		}

		result.GetCmdList()->SetGraphicsRootSignature(*root_signature_);
		result.GetCmdList()->SetComputeRootSignature(*root_signature_);

		result.GetCmdList()->SetDescriptorHeaps(1, descriptor_heap_.heap_.InitAddress());

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

		result.backbuffers_.Resize(num_backbuffers);
		for (i32 i = 0; i < num_backbuffers; i++) {
			verify_hr(result.swapchain_->GetBuffer(i, IID_PPV_ARGS(result.backbuffers_[i].InitAddress())));
		}

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

	Pipeline Device::CreateComputePipeline(D3D12_SHADER_BYTECODE bytecode)
	{
		Pipeline result;

		D3D12_COMPUTE_PIPELINE_STATE_DESC pipeline_desc{};
		pipeline_desc.pRootSignature = *root_signature_;
		pipeline_desc.CS = bytecode;

		verify_hr(device_->CreateComputePipelineState(&pipeline_desc, IID_PPV_ARGS(result.pipeline_.InitAddress())));

		return result;
	}

	ID3D12GraphicsCommandList* Encoder::GetCmdList()
	{
		return static_cast<ID3D12GraphicsCommandList*>(*cmd_list_);
	}

	void Encoder::SetPass(Pass* pass) {
		TransitionGraph& graph = device_->graph_;

		Array<D3D12_RESOURCE_BARRIER> barriers;

		for (Attachment a : pass->attachments_) {
			assert(graph.last_transitioned_state_.Contains(a.subresource));

			D3D12_RESOURCE_STATES state_before = graph.last_transitioned_state_.At(a.subresource);
			if (state_before == a.state) {
				continue;
			}

			barriers.PushBack({
				.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
				.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
				.Transition = {
					.pResource = a.subresource.resource,
					.Subresource = a.subresource.subresource,
					.StateBefore = state_before,
					.StateAfter = a.state
				}
				});

			graph.last_transitioned_state_.Insert(a.subresource, a.state);
		}

		if (barriers.Size()) {
			GetCmdList()->ResourceBarrier(static_cast<u32>(barriers.Size()), barriers.Data());
		}
	}

	i64 DescriptorHeap::AllocateTable(i64 len) {
		if (next_slot_ + len <= max_slots_) {
			i64 offset = next_slot_;
			next_slot_ += len;
			return offset;
		}

		// TODO: check if gpu is done
		next_slot_ = len;
		return 0;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Encoder::ReserveComputeSlot(DescriptorType type, i32 slot_index) {
		assert(slot_index < 8);
		DescriptorHeap& descriptor_heap = device_->descriptor_heap_;

		if (!descriptor_heap.dirty_compute_uavs_.AnyBitSet()) {
			descriptor_heap.current_compute_uavs_offset_ = descriptor_heap.AllocateTable(8);
		}

		descriptor_heap.dirty_compute_uavs_.SetBit(slot_index, true);

		D3D12_CPU_DESCRIPTOR_HANDLE handle = descriptor_heap.heap_->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += (descriptor_heap.current_compute_uavs_offset_ + slot_index) * descriptor_heap.increment_;
		return handle;
	}

	void Encoder::SetComputeDescriptors() {
		DescriptorHeap& descriptor_heap = device_->descriptor_heap_;

		D3D12_CPU_DESCRIPTOR_HANDLE heap_start = descriptor_heap.heap_->GetCPUDescriptorHandleForHeapStart();
		D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D
		};

		if (descriptor_heap.dirty_compute_uavs_.AnyBitSet()) {
			for (i32 i = 0; i < 8; i++) {
				if (!descriptor_heap.dirty_compute_uavs_.GetBit(i)) {
					D3D12_CPU_DESCRIPTOR_HANDLE handle = heap_start;
					handle.ptr += descriptor_heap.increment_ * (i + descriptor_heap.current_compute_uavs_offset_);
					device_->device_->CreateUnorderedAccessView(nullptr, nullptr, &uav_desc, handle);
				}
			}

			D3D12_GPU_DESCRIPTOR_HANDLE handle = descriptor_heap.heap_->GetGPUDescriptorHandleForHeapStart();
			handle.ptr += descriptor_heap.increment_ * (descriptor_heap.current_compute_uavs_offset_);
			GetCmdList()->SetComputeRootDescriptorTable(1, handle);
			descriptor_heap.dirty_compute_uavs_.ClearAll();
		}
	}

	void Encoder::Submit() {
		verify_hr(GetCmdList()->Close());
		ID3D12CommandList* cmd_list = GetCmdList();
		device_->cmd_queue_->ExecuteCommandLists(1, &cmd_list);
		device_->cmd_allocators_.PushBackRvalueRef(std::move(cmd_allocator_));
		device_->cmd_lists_.PushBackRvalueRef(std::move(cmd_list_));
		device_->AdvanceFence();

		device_ = nullptr;
	}

	template<typename T, typename ...Args> Box<T> MakeBox(Args&&... args) {
		return Box<T>{new T{ std::forward<Args>(args)... }};
	}

	void TransitionGraph::SetState(SubresourceDesc subresource, D3D12_RESOURCE_STATES state) {
		last_transitioned_state_.Insert(subresource, state);
	}

	Pass* TransitionGraph::AddSubsequentPass(PassAttachments attachments) {
		Box<Node> node = MakeBox<Node>();
		node->pass = MakeBox<Pass>();
		node->pass->attachments_ = std::move(attachments.attachments_);

		if (!root_) {
			root_ = node.get();
		}

		if (!tail_) {
			tail_ = node.get();
		}
		else {
			tail_->edge = node.get();
		}

		Pass* pass = node->pass.get();
		node_index_.Insert(node.get(), static_cast<i32>(node_index_.Size()));
		pending_nodes_.PushBackRvalueRef(std::move(node));

		return pass;
	}

	void Waitable::Wait() {
		HANDLE event = CreateEvent(nullptr, false, false, nullptr);
		verify_hr(fence_->SetEventOnCompletion(fence_value_, event));
		verify(WaitForSingleObject(event, INFINITE) == WAIT_OBJECT_0);
		CloseHandle(event);
	}
}

namespace Hash
{
	template<> u64 HashValue(Gfx::SubresourceDesc r) {
		u64 hash = HashMemory(&r.resource, sizeof(r.resource));
		return HashMemoryWithSeed(&r.subresource, sizeof(r.subresource), hash);
	}
}
