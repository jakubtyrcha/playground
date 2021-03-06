#include "pch.h"

#include "gfx.h"
#include "os.h"

#define DX12_ENABLE_DEBUG_LAYER _DEBUG

namespace Playground {

namespace Gfx {
    bool SubresourceDesc::operator==(SubresourceDesc other) const
    {
        return resource == other.resource && subresource == other.subresource;
    }

    PassAttachments& PassAttachments::Attach(SubresourceDesc subresource, D3D12_RESOURCE_STATES state)
    {
        attachments_.PushBack({ .subresource = subresource, .state = state });
        return *this;
    }

    IDXGIAdapter1* FindAdapter(IDXGIFactory4* dxgi_factory)
    {
        IDXGIAdapter1* adapter = nullptr;
        int adapter_index = 0;
        bool adapter_found = false;
        while (dxgi_factory->EnumAdapters1(adapter_index, &adapter) != DXGI_ERROR_NOT_FOUND) {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.VendorId == 5140) {
                desc.Flags = DXGI_ADAPTER_FLAG_SOFTWARE;
            }

            if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0) {
                HRESULT hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), nullptr);
                if (SUCCEEDED(hr)) {
                    adapter_found = true;
                    break;
                }
            }
            adapter->Release();
            adapter_index++;
        }
        plgr_assert(adapter_found);

        return adapter;
    }

    Device::Device()
    {
        verify_hr(CreateDXGIFactory1(IID_PPV_ARGS(dxgi_factory_.InitAddress())));

        *adapter_.InitAddress() = FindAdapter(dxgi_factory_.Get());

#ifdef DX12_ENABLE_DEBUG_LAYER
        ID3D12Debug* d3d12_debug = NULL;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&d3d12_debug)))) {
            d3d12_debug->EnableDebugLayer();
            d3d12_debug->Release();
        }
#endif

        D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_12_1;
        verify_hr(D3D12CreateDevice(adapter_.Get(), feature_level, IID_PPV_ARGS(device_.InitAddress())));

        D3D12MA::ALLOCATOR_DESC allocator_desc {};
        allocator_desc.pAdapter = *adapter_;
        allocator_desc.pDevice = *device_;
        verify_hr(D3D12MA::CreateAllocator(&allocator_desc, &allocator_));

        D3D12_COMMAND_QUEUE_DESC queue_desc {};
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

        D3D12_VERSIONED_ROOT_SIGNATURE_DESC root_signature_desc {};

        root_signature_desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
        root_signature_desc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

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

        root_signature_desc.Desc_1_1.NumStaticSamplers = 2;
        D3D12_STATIC_SAMPLER_DESC static_samplers[2];
        static_samplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        static_samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        static_samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        static_samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        static_samplers[0].MipLODBias = 0.f;
        static_samplers[0].MaxAnisotropy = 0;
        static_samplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        static_samplers[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        static_samplers[0].MinLOD = 0.f;
        static_samplers[0].MaxLOD = 0.f;
        static_samplers[0].ShaderRegister = 0;
        static_samplers[0].RegisterSpace = 0;
        static_samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        static_samplers[1].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        static_samplers[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        static_samplers[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        static_samplers[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        static_samplers[1].MipLODBias = 0.f;
        static_samplers[1].MaxAnisotropy = 0;
        static_samplers[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        static_samplers[1].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        static_samplers[1].MinLOD = 0.f;
        static_samplers[1].MaxLOD = 0.f;
        static_samplers[1].ShaderRegister = 1;
        static_samplers[1].RegisterSpace = 0;
        static_samplers[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        root_signature_desc.Desc_1_1.pStaticSamplers = static_samplers;

        root_signature_desc.Desc_1_1.NumParameters = _countof(params);
        root_signature_desc.Desc_1_1.pParameters = params;

        ID3DBlob* serialized_root_sig = nullptr;
        ID3DBlob* root_sig_errors = nullptr;
        HRESULT hr = D3D12SerializeVersionedRootSignature(&root_signature_desc, &serialized_root_sig, &root_sig_errors);
        if (root_sig_errors) {
            fmt::print("RootSignature serialisation errors: {}", static_cast<const char*>(root_sig_errors->GetBufferPointer()));
        }
        assert_hr(hr);

        verify_hr(device_->CreateRootSignature(0, serialized_root_sig->GetBufferPointer(), serialized_root_sig->GetBufferSize(), IID_PPV_ARGS(root_signature_.InitAddress())));

        {
            manual_descriptor_heap_.Init(*device_, { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE });
        }

        {
            manual_rtv_descriptor_heap_.Init(*device_, { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV, .NumDescriptors = 10000, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE });
        }

        {
            manual_dsv_descriptor_heap_.Init(*device_, { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV, .NumDescriptors = 10000, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE });
        }
        
        {
            frame_descriptor_heap_.Init(*device_, { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE });
        }

        {
            descriptor_heap_.Init(*device_, { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, .NumDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE });

            descriptor_heap_.graphics_tables.srv.Resize(8);
            descriptor_heap_.graphics_tables.uav.Resize(8);
            descriptor_heap_.graphics_tables.cbv.Resize(8);

            descriptor_heap_.compute_tables.srv.Resize(8);
            descriptor_heap_.compute_tables.uav.Resize(8);
            descriptor_heap_.compute_tables.cbv.Resize(8);
        }

        //

        {
            rtvs_descriptor_heap_.Init(*device_, { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV, .NumDescriptors = 4096 });
        }

        //

        {
            dsvs_descriptor_heap_.Init(*device_, { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV, .NumDescriptors = 4096 });
        }

        waitables_pool_.Resize(4096);
    }

    D3D12MA::Allocator* g_allocator = nullptr;

    Device::~Device()
    {
        plgr_assert(g_allocator == nullptr);
        g_allocator = allocator_;
        allocator_ = nullptr;
    }

    void Device::AdvanceFence()
    {
        verify_hr(cmd_queue_->Signal(*fence_, ++fence_value_));
    }

    Waitable Device::GetWaitable()
    {
        Waitable result = ReserveWaitable();
        TriggerWaitable(result, fence_value_);
        return result;
    }

    Waitable Device::ReserveWaitable()
    {
        for (i32 i = 0; i < waitables_pool_.Size(); i++) {
            if (!waitables_pool_[i].pending) {
                waitables_pool_[i].pending = true;
                Waitable result = { .device_ = this, .handle_ = i, .generation_ = waitables_pool_[i].generation };
                return result;
            }
        }

        // if we get here, the pool needs to be growable
        DEBUG_UNREACHABLE(gfx_module {});
        return {};
    }

    void Device::TriggerWaitable(Waitable waitable, u64 value)
    {
        plgr_assert(!waitables_pool_[waitable.handle_].value);
        plgr_assert(waitables_pool_[waitable.handle_].pending);
        plgr_assert(waitables_pool_[waitable.handle_].generation == waitable.generation_);
        waitables_pool_[waitable.handle_].value = value;
        waitables_pending_.PushBack(waitable);
    }

    void Device::Wait(Waitable waitable)
    {
        if (waitable.generation_ < waitables_pool_[waitable.handle_].generation) {
            return;
        }

        plgr_assert(waitable.generation_ == waitables_pool_[waitable.handle_].generation);

        HANDLE event = CreateEvent(nullptr, false, false, nullptr);
        verify_hr(fence_->SetEventOnCompletion(*waitables_pool_[waitable.handle_].value, event));
        plgr_verify(WaitForSingleObject(event, INFINITE) == WAIT_OBJECT_0);
        CloseHandle(event);
    }

    bool Device::IsDone(Waitable waitable)
    {
        if (waitable.generation_ < waitables_pool_[waitable.handle_].generation) {
            return true;
        }

        plgr_assert(waitable.generation_ == waitables_pool_[waitable.handle_].generation);
        plgr_assert(waitables_pool_[waitable.handle_].pending);

        return fence_->GetCompletedValue() >= *waitables_pool_[waitable.handle_].value;
    }

    void Device::ReleaseWhenCurrentFrameIsDone(Resource&& resource)
    {
        if (release_sets_queue_.Size() == 0) {
            release_sets_queue_.PushBackRvalueRef({});
        }
        release_sets_queue_.Last().resources.PushBackRvalueRef(std::move(resource));
    }

    void Device::ReleaseWhenCurrentFrameIsDone(DescriptorHandle h)
    {
        if (release_sets_queue_.Size() == 0) {
            release_sets_queue_.PushBackRvalueRef({});
        }
        release_sets_queue_.Last().handles.PushBack(h);
    }

    void Device::Release(DescriptorHandle h, DescriptorHandleType type)
    {
        if (type == DescriptorHandleType::Rtv) {
            i32 index = static_cast<i32>((h.handle - manual_rtv_descriptor_heap_.heap_->GetCPUDescriptorHandleForHeapStart().ptr) / manual_rtv_descriptor_heap_.increment_);
            manual_rtv_descriptor_heap_freelist_.Free(index);
        } else if (type == DescriptorHandleType::Dsv) {
            i32 index = static_cast<i32>((h.handle - manual_dsv_descriptor_heap_.heap_->GetCPUDescriptorHandleForHeapStart().ptr) / manual_dsv_descriptor_heap_.increment_);
            manual_dsv_descriptor_heap_freelist_.Free(index);
        } else if (type == DescriptorHandleType::Srv) {
            i32 index = static_cast<i32>((h.handle - manual_descriptor_heap_.heap_->GetCPUDescriptorHandleForHeapStart().ptr) / manual_descriptor_heap_.increment_);
            manual_descriptor_heap_freelist_.Free(index);
        } else {
            plgr_assert(0);
        }
    }

    Encoder Device::CreateEncoder()
    {
        Encoder result {};
        result.device_ = this;

        if (cmd_allocators_.Size()) {
            result.cmd_allocator_ = cmd_allocators_.PopBack();
        } else {
            verify_hr(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(result.cmd_allocator_.InitAddress())));
            verify_hr(result.cmd_allocator_->Reset());
        }

        if (cmd_lists_.Size()) {
            result.cmd_list_ = cmd_lists_.PopBack();
            verify_hr(result.GetCmdList()->Reset(*result.cmd_allocator_, nullptr));
        } else {
            verify_hr(device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, *result.cmd_allocator_, nullptr, IID_PPV_ARGS(result.cmd_list_.InitAddress())));
        }

        result.GetCmdList()->SetGraphicsRootSignature(*root_signature_);
        result.GetCmdList()->SetComputeRootSignature(*root_signature_);

        result.GetCmdList()->SetDescriptorHeaps(1, descriptor_heap_.heap_.InitAddress());

        return result;
    }

    Swapchain* Device::CreateSwapchain(Os::Window* window, i32 backbuffers_num)
    {
        swapchains_.PushBackRvalueRef(MakeBox<Swapchain>());
        Swapchain* result = swapchains_.Last().get();

        result->device_ = this;
        result->window_ = window;
        result->backbuffers_num_ = backbuffers_num;

        result->Recreate();

        plgr_assert(!window->swapchain_);
        window->swapchain_ = result;

        return result;
    }

    DescriptorHandle Device::CreateDescriptor(Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC const& srv_desc, Lifetime lifetime)
    {
        plgr_assert(lifetime == Lifetime::Manual); // because used with manual_descriptor_heap_

        D3D12_CPU_DESCRIPTOR_HANDLE handle = manual_descriptor_heap_.heap_->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += manual_descriptor_heap_freelist_.Allocate() * manual_descriptor_heap_.increment_;

        device_->CreateShaderResourceView(*resource->resource_, &srv_desc, handle);

        return { handle.ptr };
    }

    DescriptorHandle Device::CreateDescriptor(Resource * resource, D3D12_RENDER_TARGET_VIEW_DESC const& desc, Lifetime lifetime)
    {
        plgr_assert(lifetime == Lifetime::Manual); // because used with frame_descriptor_heap_

        D3D12_CPU_DESCRIPTOR_HANDLE handle = manual_rtv_descriptor_heap_.heap_->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += manual_rtv_descriptor_heap_freelist_.Allocate() * manual_rtv_descriptor_heap_.increment_;

        device_->CreateRenderTargetView(*resource->resource_, &desc, handle);

        return { handle.ptr };
    }

    DescriptorHandle Device::CreateDescriptor(D3D12_CONSTANT_BUFFER_VIEW_DESC const& cbv_desc, Lifetime lifetime)
    {
        plgr_assert(lifetime == Lifetime::Frame); // because used with frame_descriptor_heap_

        D3D12_CPU_DESCRIPTOR_HANDLE handle = frame_descriptor_heap_.heap_->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += frame_descriptor_heap_.AllocateTable(1) * frame_descriptor_heap_.increment_;

        device_->CreateConstantBufferView(&cbv_desc, handle);

        return { handle.ptr };
    }

    DescriptorHandle Device::CreateDescriptor(Resource * resource, D3D12_DEPTH_STENCIL_VIEW_DESC const& desc, Lifetime lifetime)
    {
        plgr_assert(lifetime == Lifetime::Manual); // because used with frame_descriptor_heap_

        D3D12_CPU_DESCRIPTOR_HANDLE handle = manual_dsv_descriptor_heap_.heap_->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += manual_dsv_descriptor_heap_freelist_.Allocate() * manual_dsv_descriptor_heap_.increment_;

        device_->CreateDepthStencilView(*resource->resource_, &desc, handle);

        return { handle.ptr };
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Device::_GetSrcHandle(DescriptorHandle handle)
    {
        return { .ptr = handle.handle };
    }

    bool IsHeapTypeStateFixed(D3D12_HEAP_TYPE heap_type)
    {
        return heap_type != D3D12_HEAP_TYPE_DEFAULT;
    }

    Resource Device::CreateBuffer(D3D12_HEAP_TYPE heap_type, i64 size, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initial_state)
    {
        Resource result {};
        result.type_ = ResourceType::Buffer;
        result.device_ = this;
        result.heap_type_ = heap_type;
        result.subresources_num_ = 1;

        D3D12_RESOURCE_DESC resource_desc = {};
        resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resource_desc.Alignment = 0;
        resource_desc.Width = size;
        resource_desc.Height = 1;
        resource_desc.DepthOrArraySize = 1;
        resource_desc.MipLevels = 1;
        resource_desc.Format = format;
        resource_desc.SampleDesc.Count = 1;
        resource_desc.SampleDesc.Quality = 0;
        resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resource_desc.Flags = flags;

        D3D12MA::ALLOCATION_DESC allocation_desc = {};
        allocation_desc.HeapType = heap_type;

        verify_hr(allocator_->CreateResource(
            &allocation_desc,
            &resource_desc,
            initial_state,
            NULL,
            result.allocation_.InitAddress(),
            IID_PPV_ARGS(result.resource_.InitAddress())));

        if (!IsHeapTypeStateFixed(result.heap_type_)) {
            graph_.SetState({ .resource = *result.resource_ }, initial_state);
        }

        return result;
    }

    Resource Device::CreateTexture1D(D3D12_HEAP_TYPE heap_type, i64 size, DXGI_FORMAT format, i32 miplevels, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initial_state)
    {
        Resource result {};
        result.type_ = ResourceType::Texture1D;
        result.device_ = this;
        result.heap_type_ = heap_type;
        result.subresources_num_ = miplevels;

        D3D12_RESOURCE_DESC resource_desc = {};
        resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
        resource_desc.Alignment = 0;
        resource_desc.Width = size;
        resource_desc.Height = 1;
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
            initial_state,
            NULL,
            result.allocation_.InitAddress(),
            IID_PPV_ARGS(result.resource_.InitAddress())));

        plgr_assert(miplevels == 1);
        if (!IsHeapTypeStateFixed(result.heap_type_)) {
            graph_.SetState({ .resource = *result.resource_ }, initial_state);
        }

        return result;
    }

    Resource Device::CreateTexture2D(D3D12_HEAP_TYPE heap_type, Vector2i size, DXGI_FORMAT format, i32 miplevels, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initial_state)
    {
        Resource result {};
        result.type_ = ResourceType::Texture2D;
        result.device_ = this;
        result.heap_type_ = heap_type;
        result.subresources_num_ = miplevels;

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

        // TODO: make this an arg
        D3D12_CLEAR_VALUE clear_value {
            .Format = format
        };

        if (!!(resource_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) {
            clear_value.DepthStencil.Depth = 1.f;
        }

        bool use_clear_value = !!(resource_desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL));

        verify_hr(allocator_->CreateResource(
            &allocation_desc,
            &resource_desc,
            initial_state,
            use_clear_value ? &clear_value : nullptr,
            result.allocation_.InitAddress(),
            IID_PPV_ARGS(result.resource_.InitAddress())));

        plgr_assert(resource_desc.DepthOrArraySize == 1);

        if (!IsHeapTypeStateFixed(result.heap_type_)) {
            for(i32 i=0; i<miplevels; i++) {
                graph_.SetState({ .resource = *result.resource_, .subresource = i }, initial_state);
            }
        }

        return result;
    }

    // done post destructor, so all the RAII resources have a chance to die
    void Device::ShutdownAllocator()
    {
        plgr_assert(g_allocator);
        g_allocator->Release();
        g_allocator = nullptr;

#ifdef DX12_ENABLE_DEBUG_LAYER
        IDXGIDebug1* dxgi_debug = NULL;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgi_debug)))) {
            dxgi_debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
            dxgi_debug->Release();
        }
#endif
    }

    Resource::~Resource()
    {
        if (resource_.Get() && !IsHeapTypeStateFixed(heap_type_)) {
            device_->graph_.Drop(*resource_);
        }
    }

    Resource& Resource::operator=(Resource&& other)
    {
        if (resource_.Get() && !IsHeapTypeStateFixed(heap_type_)) {
            device_->graph_.Drop(*resource_);
        }

        type_ = other.type_;
        device_ = other.device_;
        subresources_num_ = other.subresources_num_;
        heap_type_ = other.heap_type_;
        resource_ = std::move(other.resource_);
        allocation_ = std::move(other.allocation_);

        return *this;
    }

    ID3D12GraphicsCommandList* Encoder::GetCmdList()
    {
        return static_cast<ID3D12GraphicsCommandList*>(*cmd_list_);
    }

    void Encoder::SetPass(Pass* pass)
    {
        TransitionGraph& graph = device_->graph_;

        Array<D3D12_RESOURCE_BARRIER> barriers;

        for (Attachment a : pass->attachments_) {
            plgr_assert(graph.last_transitioned_state_.Contains(a.subresource));

            D3D12_RESOURCE_STATES state_before = graph.last_transitioned_state_.At(a.subresource);
            if (state_before == a.state) {
                continue;
            }

            barriers.PushBack({ .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
                .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
                .Transition = {
                    .pResource = a.subresource.resource,
                    .Subresource = As<u32>(a.subresource.subresource),
                    .StateBefore = state_before,
                    .StateAfter = a.state } });

            graph.last_transitioned_state_.Insert(a.subresource, a.state);
        }

        if (barriers.Size()) {
            GetCmdList()->ResourceBarrier(static_cast<u32>(barriers.Size()), barriers.Data());
        }
    }

    void DescriptorHeap::Init(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_DESC heap_desc)
    {
        verify_hr(device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(heap_.InitAddress())));

        increment_ = device->GetDescriptorHandleIncrementSize(heap_desc.Type);
        max_slots_ = heap_desc.NumDescriptors;
    }

    void DescriptorHeap::FenceDescriptors(Waitable waitable)
    {
        fences_.PushBack({ .offset = next_slot_, .waitable = waitable });

        while (fences_.Size() && fences_.First().waitable.IsDone()) {
            used_start_slot_ = fences_.RemoveAt(0).offset;
        }
    }

    void DescriptorHeap::Table::Resize(i32 size)
    {
        dirty.Resize(size);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::ReserveTableSlot(Table& table, i32 slot_index)
    {
        if (!table.dirty.AnyBitSet()) {
            table.offset = AllocateTable(8);
        }

        table.dirty.SetBit(slot_index, true);

        D3D12_CPU_DESCRIPTOR_HANDLE handle = heap_->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += (table.offset + slot_index) * increment_;
        return handle;
    }

    i64 DescriptorHeap::AllocateTable(i64 len)
    {
        plgr_assert((next_slot_ >= used_start_slot_) || (next_slot_ + len < used_start_slot_));

        if (next_slot_ + len <= max_slots_) {
            i64 offset = next_slot_;
            next_slot_ += len;
            return offset;
        }

        plgr_assert((len < used_start_slot_));

        next_slot_ = len;
        return 0;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Encoder::ReserveGraphicsSlot(DescriptorType type, i32 slot_index)
    {
        plgr_assert(slot_index < 8);

        DescriptorHeap& descriptor_heap = device_->descriptor_heap_;

        if (type == DescriptorType::SRV) {
            return descriptor_heap.ReserveTableSlot(descriptor_heap.graphics_tables.srv, slot_index);
        } else if (type == DescriptorType::UAV) {
            return descriptor_heap.ReserveTableSlot(descriptor_heap.graphics_tables.uav, slot_index);
        } else if (type == DescriptorType::CBV) {
            return descriptor_heap.ReserveTableSlot(descriptor_heap.graphics_tables.cbv, slot_index);
        } else {
            DEBUG_UNREACHABLE(gfx_module {});
            return {};
        }
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Encoder::ReserveComputeSlot(DescriptorType type, i32 slot_index)
    {
        plgr_assert(slot_index < 8);
        DescriptorHeap& descriptor_heap = device_->descriptor_heap_;

        if (type == DescriptorType::SRV) {
            return descriptor_heap.ReserveTableSlot(descriptor_heap.compute_tables.srv, slot_index);
        } else if (type == DescriptorType::UAV) {
            return descriptor_heap.ReserveTableSlot(descriptor_heap.compute_tables.uav, slot_index);
        } else if (type == DescriptorType::CBV) {
            return descriptor_heap.ReserveTableSlot(descriptor_heap.compute_tables.cbv, slot_index);
        } else {
            DEBUG_UNREACHABLE(gfx_module {});
            return {};
        }
    }

    void Encoder::SetGraphicsDescriptor(DescriptorType type, i32 slot_index, DescriptorHandle handle)
    {
        device_->device_->CopyDescriptorsSimple(1, ReserveGraphicsSlot(type, slot_index), device_->_GetSrcHandle(handle), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    template <typename F, typename F1>
    void DescriptorHeap::SetRootTable(Table& table, F fill_null_slot, F1 set_root_param)
    {
        if (!table.dirty.AnyBitSet()) {
            return;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE heap_start = heap_->GetCPUDescriptorHandleForHeapStart();
        for (i32 i = 0; i < 8; i++) {
            if (!table.dirty.GetBit(i)) {
                D3D12_CPU_DESCRIPTOR_HANDLE handle = heap_start;
                handle.ptr += increment_ * (i + table.offset);
                fill_null_slot(handle);
            }
        }

        D3D12_GPU_DESCRIPTOR_HANDLE handle = heap_->GetGPUDescriptorHandleForHeapStart();
        handle.ptr += increment_ * (table.offset);

        table.dirty.ClearAll();

        set_root_param(handle);
    }

    void Encoder::SetGraphicsDescriptors()
    {
        DescriptorHeap& descriptor_heap = device_->descriptor_heap_;

        descriptor_heap.SetRootTable(
            descriptor_heap.graphics_tables.srv, [this](D3D12_CPU_DESCRIPTOR_HANDLE handle) {
			D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{
				.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
			};
			device_->device_->CreateShaderResourceView(nullptr, &srv_desc, handle); }, [this](D3D12_GPU_DESCRIPTOR_HANDLE handle) { GetCmdList()->SetGraphicsRootDescriptorTable(0, handle); });

        descriptor_heap.SetRootTable(
            descriptor_heap.graphics_tables.uav, [this](D3D12_CPU_DESCRIPTOR_HANDLE handle) {
			D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{
				.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
				.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D
			};
			device_->device_->CreateUnorderedAccessView(nullptr, nullptr, &uav_desc, handle); }, [this](D3D12_GPU_DESCRIPTOR_HANDLE handle) { GetCmdList()->SetGraphicsRootDescriptorTable(1, handle); });

        descriptor_heap.SetRootTable(
            descriptor_heap.graphics_tables.cbv, [this](D3D12_CPU_DESCRIPTOR_HANDLE handle) {
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc{
				.BufferLocation = 0,
				.SizeInBytes = 0
			};
			device_->device_->CreateConstantBufferView(&cbv_desc, handle); }, [this](D3D12_GPU_DESCRIPTOR_HANDLE handle) { GetCmdList()->SetGraphicsRootDescriptorTable(2, handle); });
    }

    void Encoder::SetComputeDescriptors()
    {
        DescriptorHeap& descriptor_heap = device_->descriptor_heap_;

        descriptor_heap.SetRootTable(
            descriptor_heap.compute_tables.srv, [this](D3D12_CPU_DESCRIPTOR_HANDLE handle) {
			D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{
				.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
			};
			device_->device_->CreateShaderResourceView(nullptr, &srv_desc, handle); }, [this](D3D12_GPU_DESCRIPTOR_HANDLE handle) { GetCmdList()->SetComputeRootDescriptorTable(0, handle); });

        descriptor_heap.SetRootTable(
            descriptor_heap.compute_tables.uav, [this](D3D12_CPU_DESCRIPTOR_HANDLE handle) {
			D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{
				.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
				.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D
			};
			device_->device_->CreateUnorderedAccessView(nullptr, nullptr, &uav_desc, handle); }, [this](D3D12_GPU_DESCRIPTOR_HANDLE handle) { GetCmdList()->SetComputeRootDescriptorTable(1, handle); });

        descriptor_heap.SetRootTable(
            descriptor_heap.compute_tables.cbv, [this](D3D12_CPU_DESCRIPTOR_HANDLE handle) {
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc{
				.BufferLocation = 0,
				.SizeInBytes = 0
			};
			device_->device_->CreateConstantBufferView(&cbv_desc, handle); }, [this](D3D12_GPU_DESCRIPTOR_HANDLE handle) { GetCmdList()->SetComputeRootDescriptorTable(2, handle); });
    }

    void Encoder::SetViewportAndScissorRect(Vector2i res)
    {
        D3D12_VIEWPORT vp {
            .Width = As<f32>(res.x()),
            .Height = As<f32>(res.y()),
            .MinDepth = 0.f,
            .MaxDepth = 1.f
        };
        GetCmdList()->RSSetViewports(1, &vp);

        const D3D12_RECT r = { 0, 0, res.x(), res.y() };
        GetCmdList()->RSSetScissorRects(1, &r);
    }

    Waitable Encoder::GetWaitable()
    {
        waitables_to_trigger_.PushBack(device_->ReserveWaitable());
        return waitables_to_trigger_.Last();
    }

    void Device::_Submit(Encoder&& encoder)
    {
        ID3D12CommandList* cmd_list = encoder.GetCmdList();
        cmd_queue_->ExecuteCommandLists(1, &cmd_list);
        AdvanceFence();
        cmd_allocators_.PushBackRvalueRef(std::move(encoder.cmd_allocator_));
        cmd_lists_.PushBackRvalueRef(std::move(encoder.cmd_list_));

        for (Waitable waitable : encoder.waitables_to_trigger_) {
            TriggerWaitable(waitable, fence_value_);
        }
        encoder.waitables_to_trigger_.Clear();
    }

    void Encoder::Submit()
    {
        verify_hr(GetCmdList()->Close());

        device_->_Submit(std::move(*this));

        device_ = nullptr;
    }

    void Device::RecycleResources()
    {
        Waitable frame_end_fence = GetWaitable();

        if (release_sets_queue_.Size()) {
            release_sets_queue_.Last().waitable = frame_end_fence;
        }
        while (release_sets_queue_.Size() && release_sets_queue_.First().waitable.IsDone()) {
            for (DescriptorHandle h : release_sets_queue_.First().handles) {
                i32 index = static_cast<i32>((h.handle - manual_descriptor_heap_.heap_->GetCPUDescriptorHandleForHeapStart().ptr) / manual_descriptor_heap_.increment_);
                manual_descriptor_heap_freelist_.Free(index);
            }

            release_sets_queue_.RemoveAt(0);
        }

        release_sets_queue_.PushBackRvalueRef({});

        descriptor_heap_.FenceDescriptors(frame_end_fence);
        frame_descriptor_heap_.FenceDescriptors(frame_end_fence);
        rtvs_descriptor_heap_.FenceDescriptors(frame_end_fence);
        dsvs_descriptor_heap_.FenceDescriptors(frame_end_fence);
        for (Com::Box<ID3D12CommandAllocator>& allocator : cmd_allocators_) {
            cmd_allocators_pending_.PushBackRvalueRef(PendingCommandAllocator { .allocator = std::move(allocator), .waitable = frame_end_fence });
        }
        cmd_allocators_.Clear();

        while (waitables_pending_.Size() && IsDone(waitables_pending_.First())) {
            i32 handle = waitables_pending_.RemoveAt(0).handle_;
            waitables_pool_[handle].pending = false;
            waitables_pool_[handle].value = {};
            waitables_pool_[handle].generation++;
        }

        while (cmd_allocators_pending_.Size() && IsDone(cmd_allocators_pending_.First().waitable)) {
            cmd_allocators_.PushBackRvalueRef(std::move(cmd_allocators_pending_.RemoveAt(0).allocator));
            verify_hr(cmd_allocators_.Last()->Reset());
        }
    }

    void TransitionGraph::SetState(SubresourceDesc subresource, D3D12_RESOURCE_STATES state)
    {
        D3D12_HEAP_PROPERTIES heap_properties;
        D3D12_HEAP_FLAGS heap_flags;
        verify_hr(subresource.resource->GetHeapProperties(&heap_properties, &heap_flags));
        plgr_assert(!IsHeapTypeStateFixed(heap_properties.Type));
        last_transitioned_state_.Insert(subresource, state);
    }

    D3D12_RESOURCE_STATES TransitionGraph::_GetCurrentState(SubresourceDesc subresource) const
    {
        return last_transitioned_state_.At(subresource);
    }

    void TransitionGraph::Drop(ID3D12Resource* ptr)
    {
        D3D12_HEAP_PROPERTIES heap_properties;
        D3D12_HEAP_FLAGS heap_flags;
        verify_hr(ptr->GetHeapProperties(&heap_properties, &heap_flags));
        plgr_assert(!IsHeapTypeStateFixed(heap_properties.Type));

        for (decltype(last_transitioned_state_)::Iterator iter = last_transitioned_state_.begin(), end = last_transitioned_state_.end(); iter != end;) {
            if (iter.Key().resource == ptr) {
                iter = last_transitioned_state_.Remove(iter.Key());
            } else {
                iter++;
            }
        }
    }

    Pass* TransitionGraph::AddSubsequentPass(PassAttachments attachments)
    {
        Box<Node> node = MakeBox<Node>();
        node->pass = MakeBox<Pass>();
        node->pass->attachments_ = std::move(attachments.attachments_);

        if (!root_) {
            root_ = node.get();
        }

        if (!tail_) {
            tail_ = node.get();
        } else {
            tail_->edge = node.get();
        }

        Pass* pass = node->pass.get();
        node_index_.Insert(node.get(), static_cast<i32>(node_index_.Size()));
        pending_nodes_.PushBackRvalueRef(std::move(node));

        return pass;
    }

    void Swapchain::Destroy()
    {
        backbuffers_.Clear();
        for(i32 i=0; i<backbuffer_descs_.Size(); i++) 
        {
            device_->Release(backbuffer_descs_[i].rtv, DescriptorHandleType::Rtv);
        }
        backbuffer_descs_.Clear();
        swapchain_->Release();
    }

    void Swapchain::Recreate()
    {
        const DXGI_FORMAT BACKBUFFER_FMT = DXGI_FORMAT_R8G8B8A8_UNORM;

        IDXGISwapChain1* swapchain1 = nullptr;
        DXGI_SWAP_CHAIN_DESC1 swapchain_desc {};
        swapchain_desc.BufferCount = backbuffers_num_;
        swapchain_desc.Width = window_->resolution_.x();
        swapchain_desc.Height = window_->resolution_.y();
        swapchain_desc.Format = BACKBUFFER_FMT;
        swapchain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchain_desc.SampleDesc.Count = 1;
        swapchain_desc.SampleDesc.Quality = 0;
        swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapchain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapchain_desc.Scaling = DXGI_SCALING_STRETCH;
        swapchain_desc.Stereo = FALSE;

        verify_hr(device_->dxgi_factory_->CreateSwapChainForHwnd(*device_->cmd_queue_, window_->hwnd_, &swapchain_desc, nullptr, nullptr, &swapchain1));

        verify_hr(swapchain1->QueryInterface<IDXGISwapChain4>(swapchain_.InitAddress()));
        swapchain1->Release();

        swapchain_->SetMaximumFrameLatency(backbuffers_num_);

        backbuffers_.Resize(backbuffers_num_);
        for (i32 i = 0; i < backbuffers_num_; i++) {
            ID3D12Resource* backbuffer;
            verify_hr(swapchain_->GetBuffer(i, IID_PPV_ARGS(&backbuffer)));
            backbuffers_[i] = Resource::From(device_, backbuffer);
        }
        for (i32 i = 0; i < backbuffers_num_; i++) {
            device_->graph_.SetState({ .resource = *backbuffers_[i].resource_ }, D3D12_RESOURCE_STATE_PRESENT);
            backbuffer_descs_.PushBack({ .resource = &backbuffers_[i], .rtv = device_->CreateDescriptor(&backbuffers_[i], D3D12_RENDER_TARGET_VIEW_DESC{
                .Format = BACKBUFFER_FMT,
                .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
                .Texture2D = {}
            }, Lifetime::Manual), .fmt = BACKBUFFER_FMT });
        }
    }

    RenderTargetDesc Swapchain::GetCurrentBackbufferAsRenderTarget()
    {
        u32 index = swapchain_->GetCurrentBackBufferIndex();
        return backbuffer_descs_[index];
    }

    void Waitable::Wait()
    {
        device_->Wait(*this);
    }

    bool Waitable::IsDone()
    {
        return device_->IsDone(*this);
    }

    Resource Resource::From(Device * device, ID3D12Resource * ptr)
    {
        Resource result;
        result.type_ = ResourceType::Texture2D;
        result.device_ = device;
        result.subresources_num_ = 1;
        result.resource_.ptr_ = ptr;

        return result;
    }

    Optional<Box<Pipeline>> Pipeline::From(Device* device, D3D12_GRAPHICS_PIPELINE_STATE_DESC const& desc)
    {
        Box<Pipeline> result = MakeBox<Pipeline>();
        if (FAILED(device->device_->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(result->pipeline_.InitAddress())))) {
            return NullOpt;
        }
        return result;
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC GetDefaultPipelineStateDesc(Device* device)
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc {};
        pso_desc.NodeMask = 1;
        pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pso_desc.pRootSignature = *device->root_signature_;
        pso_desc.SampleMask = UINT_MAX;
        //pso_desc.NumRenderTargets = 2;
        //pso_desc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
        //pso_desc.RTVFormats[1] = DXGI_FORMAT_R16G16_FLOAT;
        //pso_desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        pso_desc.SampleDesc.Count = 1;
        pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

        //pso_desc.VS = GetShaderFromShaderFileSource({ .file_path = L"../data/sphere_splatting.hlsl",
        //                                                .entrypoint = L"VsMain",
        //                                                .profile = L"vs_6_0" })
        //                  ->GetBytecode();
        //
        //pso_desc.PS = GetShaderFromShaderFileSource({ .file_path = L"../data/sphere_splatting.hlsl",
        //                                                .entrypoint = L"PsMain",
        //                                                .profile = L"ps_6_0" })
        //                  ->GetBytecode();

        // Create the blending setup
        {
            D3D12_BLEND_DESC& desc = pso_desc.BlendState;
            desc.AlphaToCoverageEnable = false;
            desc.RenderTarget[0].BlendEnable = false;
            desc.RenderTarget[0].LogicOpEnable = false;
            desc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        }

        // Create the rasterizer state
        {
            D3D12_RASTERIZER_DESC& desc = pso_desc.RasterizerState;
            desc.FillMode = D3D12_FILL_MODE_SOLID;
            desc.CullMode = D3D12_CULL_MODE_NONE;
            desc.FrontCounterClockwise = FALSE;
            desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
            desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            desc.DepthClipEnable = true;
            desc.MultisampleEnable = FALSE;
            desc.AntialiasedLineEnable = FALSE;
            desc.ForcedSampleCount = 0;
            desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        }

        // Create depth-stencil State
        {
            D3D12_DEPTH_STENCIL_DESC& desc = pso_desc.DepthStencilState;
            desc.DepthEnable = false;
            desc.StencilEnable = false;
        }

        return pso_desc;
    }

    void UpdateTexture2DSubresource(Device* device, Resource* resource, i32 subresource, Vector2i resource_size, DXGI_FORMAT fmt, const void* src, i64 src_pitch, i32 rows)
    {
        i64 upload_pitch = AlignedForward(src_pitch, As<i64>(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
        i64 upload_size = upload_pitch * rows;

        Resource upload_buffer = device->CreateBuffer(
            D3D12_HEAP_TYPE_UPLOAD,
            upload_size,
            DXGI_FORMAT_UNKNOWN,
            D3D12_RESOURCE_FLAG_NONE,
            D3D12_RESOURCE_STATE_GENERIC_READ);

        u8* mapped_memory = nullptr;
        verify_hr(upload_buffer.resource_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_memory)));
        for (i64 r = 0; r < resource_size.y(); r++) {
            memcpy(mapped_memory + r * upload_pitch, reinterpret_cast<const u8*>(src) + r * src_pitch, src_pitch);
        }

        upload_buffer.resource_->Unmap(0, nullptr);

        Encoder encoder = device->CreateEncoder();
        D3D12_TEXTURE_COPY_LOCATION copy_src {
            .pResource = *upload_buffer.resource_,
            .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
            .PlacedFootprint = {
                .Footprint = {
                    .Format = fmt,
                    .Width = static_cast<u32>(resource_size.x()),
                    .Height = static_cast<u32>(resource_size.y()),
                    .Depth = 1,
                    .RowPitch = static_cast<u32>(upload_pitch) } }
        };
        D3D12_TEXTURE_COPY_LOCATION copy_dst {
            .pResource = *resource->resource_,
            .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
            .SubresourceIndex = As<u32>(subresource)
        };

        Pass* copy_to_pass = device->graph_.AddSubsequentPass(Gfx::PassAttachments {}.Attach({ .resource = *resource->resource_, .subresource = subresource }, D3D12_RESOURCE_STATE_COPY_DEST));
        encoder.SetPass(copy_to_pass);
        encoder.GetCmdList()->CopyTextureRegion(&copy_dst, 0, 0, 0, &copy_src, nullptr);

        Pass* transition_to_readable_pass = device->graph_.AddSubsequentPass(Gfx::PassAttachments {}.Attach({ .resource = *resource->resource_, .subresource = subresource }, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
        encoder.SetPass(transition_to_readable_pass);
        encoder.Submit();

        device->ReleaseWhenCurrentFrameIsDone(std::move(upload_buffer));
    }

    void UpdateBuffer(Device* device, Resource* resource, const void* src, i64 bytes, Optional<D3D12_RESOURCE_STATES> post_transition)
    {
        i64 upload_pitch = AlignedForward(bytes, As<i64>(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
        i64 upload_size = bytes;

        Gfx::Resource upload_buffer = device->CreateBuffer(
            D3D12_HEAP_TYPE_UPLOAD,
            upload_size,
            DXGI_FORMAT_UNKNOWN,
            D3D12_RESOURCE_FLAG_NONE,
            D3D12_RESOURCE_STATE_GENERIC_READ);

        u8* mapped_memory = nullptr;
        verify_hr(upload_buffer.resource_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_memory)));
        memcpy(mapped_memory, src, bytes);
        upload_buffer.resource_->Unmap(0, nullptr);

        Gfx::Encoder encoder = device->CreateEncoder();

        Gfx::Pass* copy_to_pass = device->graph_.AddSubsequentPass(Gfx::PassAttachments {}.Attach({ .resource = *resource->resource_ }, D3D12_RESOURCE_STATE_COPY_DEST));
        encoder.SetPass(copy_to_pass);
        encoder.GetCmdList()->CopyBufferRegion(*resource->resource_, 0, *upload_buffer.resource_, 0, bytes);

        if (post_transition) {
            Gfx::Pass* transition_to_readable_pass = device->graph_.AddSubsequentPass(Gfx::PassAttachments {}.Attach({ .resource = *resource->resource_ }, *post_transition));
            encoder.SetPass(transition_to_readable_pass);
        }
        encoder.Submit();

        device->ReleaseWhenCurrentFrameIsDone(std::move(upload_buffer));
    }

    DescriptorHandle MakeFrameCbv(Device* device, Resource* resource, i32 bytes)
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc {
            .BufferLocation = resource->resource_->GetGPUVirtualAddress(),
            .SizeInBytes = static_cast<u32>(AlignedForward(bytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT))
        };

        return device->CreateDescriptor(cbv_desc, Lifetime::Frame);
    }

    DescriptorHandle MakeFrameCbvFromMemory(Device* device, void const* src, i32 bytes)
    {
        Resource cbuffer = device->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, AlignedForward(bytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ);

        void* mapped_ptr = nullptr;
        verify_hr(cbuffer.resource_->Map(0, nullptr, &mapped_ptr));
        memcpy(mapped_ptr, src, bytes);
        cbuffer.resource_->Unmap(0, nullptr);

        DescriptorHandle result = MakeFrameCbv(device, &cbuffer, bytes);

        device->ReleaseWhenCurrentFrameIsDone(std::move(cbuffer));

        return result;
    }
}

namespace Hash {
    template <>
    u64 HashValue(Gfx::SubresourceDesc r)
    {
        u64 hash = HashMemory(&r.resource, sizeof(r.resource));
        return HashMemoryWithSeed(&r.subresource, sizeof(r.subresource), hash);
    }
}

}