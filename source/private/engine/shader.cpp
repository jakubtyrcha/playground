#include "pch.h"

#include "Gfx.h"
#include "Hash.h"
#include "Shader.h"
#include <dxcapi.h>
#include <dxil/inc/dxcapi.use.h>

namespace Playground {
namespace Gfx {
    struct Compiler {
        dxc::DxcDllSupport dxc_dll_support_;
        IDxcCompiler* compiler_ = nullptr;
        IDxcLibrary* library_ = nullptr;
        IDxcIncludeHandler* include_handler_ = nullptr;

        Compiler()
        {
            dxc_dll_support_.Initialize();

            verify_hr(dxc_dll_support_.CreateInstance(CLSID_DxcCompiler, &compiler_));
            verify_hr(dxc_dll_support_.CreateInstance(CLSID_DxcLibrary, &library_));
            verify_hr(library_->CreateIncludeHandler(&include_handler_));
        }

        ~Compiler()
        {
        }

        static Compiler& GetInstance()
        {
            static Compiler instance;
            return instance;
        }

        IDxcBlob* PreprocessShader(
            IDxcBlob* source,
            const wchar_t* file_name)
        {
            IDxcOperationResult* operation_result;

            HRESULT hr = compiler_->Preprocess(source, file_name, nullptr, 0,
                nullptr, 0, include_handler_, &operation_result);
            if (SUCCEEDED(hr)) {
                // TODO: print warning
                ID3DBlob* error_messages = nullptr;
                if (SUCCEEDED(operation_result->GetErrorBuffer((IDxcBlobEncoding**)&error_messages))) {
                    if (error_messages->GetBufferPointer()) {
                        fmt::print("Compiled shader with warnings: {}", (const char*)error_messages->GetBufferPointer());
                    }

                    error_messages->Release();
                }

                IDxcBlob* bytecode;
                verify_hr(operation_result->GetResult((IDxcBlob**)&bytecode));
                operation_result->Release();
                return bytecode;
            } else {
                ID3DBlob* error_messages;
                verify_hr(operation_result->GetErrorBuffer((IDxcBlobEncoding**)&error_messages));
                fmt::print("Failed to compile shader: {}", (const char*)error_messages->GetBufferPointer());
                error_messages->Release();
                operation_result->Release();

                return nullptr;
            }

            DEBUG_UNREACHABLE(default_module {});
            return nullptr;
        }

        IDxcBlob* CompileShader(
            IDxcBlob* source,
            const wchar_t* file_name,
            const wchar_t* entrypoint,
            const wchar_t* target)
        {
            IDxcOperationResult* operation_result;

            HRESULT hr = compiler_->Compile(source, file_name, entrypoint, target, nullptr, 0,
                nullptr, 0, include_handler_, &operation_result);
            if (SUCCEEDED(hr)) {
                // TODO: print warning
                ID3DBlob* error_messages = nullptr;
                if (SUCCEEDED(operation_result->GetErrorBuffer((IDxcBlobEncoding**)&error_messages))) {
                    if (error_messages->GetBufferPointer()) {
                        fmt::print("Compiled shader with warnings: {}", (const char*)error_messages->GetBufferPointer());
                    }

                    error_messages->Release();
                }

                IDxcBlob* bytecode;
                verify_hr(operation_result->GetResult((IDxcBlob**)&bytecode));
                operation_result->Release();
                return bytecode;
            } else {
                ID3DBlob* error_messages;
                verify_hr(operation_result->GetErrorBuffer((IDxcBlobEncoding**)&error_messages));
                fmt::print("Failed to compile shader: {}", (const char*)error_messages->GetBufferPointer());
                error_messages->Release();
                operation_result->Release();

                return nullptr;
            }

            DEBUG_UNREACHABLE(default_module {});
            return nullptr;
        }
    };

    IDxcBlob* Wrap(const char* ptr, i64 len);

    struct WrappedBlob : public IDxcBlob {
        const char* ptr_ = nullptr;
        u64 size_ = 0;
        u32 refs_ = 1;

        WrappedBlob(const char* ptr, u64 size)
            : ptr_(ptr)
            , size_(size)
        {
        }

        HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override
        {
            return E_NOTIMPL;
        }

        ULONG __stdcall AddRef(void) override
        {
            return refs_++;
        }

        ULONG __stdcall Release(void) override
        {
            u32 result = --refs_;
            if (result == 0) {
                delete this;
            }
            return result;
        }

        LPVOID __stdcall GetBufferPointer(void) override
        {
            return const_cast<void*>(reinterpret_cast<const void*>(ptr_));
        }

        virtual SIZE_T __stdcall GetBufferSize(void) override
        {
            return size_;
        }
    };

    IDxcBlob* Wrap(const char* ptr, i64 len)
    {
        return new WrappedBlob(ptr, len);
    }

    //

    ShaderReloadResult IPipelineBuilder::BeginRecreate()
    {
        plgr_assert(!pending_pipeline_);

        bool changed = false;
        for (IShaderSource* source : shaders_) {
            if (source->IsTransitionPending()) {
                changed = true;
                break;
            }
        }

        if (!changed) {
            return ShaderReloadResult::NoChange;
        }

        pending_pipeline_ = Build();

        if (!pending_pipeline_) {
            return ShaderReloadResult::RecompileFail;
        }

        return ShaderReloadResult::RecompileOk;
    }

    void IPipelineBuilder::CommitRecreate()
    {
        plgr_assert(pending_pipeline_);
        pipeline_ = std::move(*pending_pipeline_);
        pending_pipeline_ = NullOpt;
    }

    void IPipelineBuilder::DropRecreate()
    {
        plgr_assert(pending_pipeline_);
        pending_pipeline_ = NullOpt;
    }

    ID3D12PipelineState* IPipelineBuilder::GetPSO()
    {
        if (!pipeline_) {
            pipeline_ = *Build();
        }

        return pipeline_->pipeline_.Get();
    }

    struct ShaderCache {
        Array<IPipelineBuilder*> pipelines_;
        Hashmap<u64, Box<IShaderSource>> shaders_;

        IShaderSource* GetShaderFromStaticSource(ShaderStaticSourceDesc);
        IShaderSource* GetShaderFromShaderFileSource(ShaderFileSourceDesc);
        void Register(IPipelineBuilder*);
        void Deregister(IPipelineBuilder*);

        void Reload();

        static ShaderCache& GetInstance();
    };

    IShaderSource* IPipelineBuilder::GetShaderFromStaticSource(ShaderStaticSourceDesc desc)
    {
        // TODO: time for a set?
        IShaderSource* shader = ShaderCache::GetInstance().GetShaderFromStaticSource(desc);
        if (!shaders_.Find(shader)) {
            shaders_.PushBack(shader);
        }
        return shader;
    }

    IShaderSource* IPipelineBuilder::GetShaderFromShaderFileSource(ShaderFileSourceDesc desc)
    {
        IShaderSource* shader = ShaderCache::GetInstance().GetShaderFromShaderFileSource(desc);
        if (!shaders_.Find(shader)) {
            shaders_.PushBack(shader);
        }
        return shader;
    }

    ShaderCache& ShaderCache::GetInstance()
    {
        static ShaderCache instance;
        return instance;
    }

    IShaderSource* ShaderCache::GetShaderFromStaticSource(ShaderStaticSourceDesc desc)
    {
        u64 hash = 0;
        hash = HashMemory(desc.source, strlen(desc.source));
        hash = HashMemoryWithSeed(desc.entrypoint, wcslen(desc.entrypoint) * sizeof(wchar_t), hash);
        hash = HashMemoryWithSeed(desc.profile, wcslen(desc.profile) * sizeof(wchar_t), hash);

        if (Optional<Box<IShaderSource>*> shader = shaders_.Find(hash); shader) {
            return (*shader)->get();
        }

        Box<IShaderSource> shader = MakeBox<StaticShaderSource>(desc);
        IShaderSource* ptr = shader.get();
        shaders_.Insert(hash, std::move(shader));

        return ptr;
    }

    IShaderSource* ShaderCache::GetShaderFromShaderFileSource(ShaderFileSourceDesc desc)
    {
        u64 hash = 0;
        hash = HashMemory(desc.file_path, wcslen(desc.file_path) * sizeof(wchar_t));
        hash = HashMemoryWithSeed(desc.entrypoint, wcslen(desc.entrypoint) * sizeof(wchar_t), hash);
        hash = HashMemoryWithSeed(desc.profile, wcslen(desc.profile) * sizeof(wchar_t), hash);

        if (Optional<Box<IShaderSource>*> shader = shaders_.Find(hash); shader) {
            return (*shader)->get();
        }

        Box<IShaderSource> shader = MakeBox<FileShaderSource>(desc);
        IShaderSource* ptr = shader.get();
        shaders_.Insert(hash, std::move(shader));

        return ptr;
    }

    void ShaderCache::Register(IPipelineBuilder* pb)
    {
        pipelines_.PushBack(pb);
    }

    void ShaderCache::Deregister(IPipelineBuilder* pb)
    {
        for (i32 i = 0; i < pipelines_.Size(); i++) {
            if (pipelines_[i] == pb) {
                pipelines_.RemoveAtAndSwapWithLast(i);
                return;
            }
        }
    }

    void ShaderCache::Reload()
    {
        // TODO: fix iteration
        //for(Box<IShaderSource>& shader : shaders_) {
        bool failed = false;

        Array<IShaderSource*> modified_shaders;

        for (decltype(shaders_)::Iterator iter = shaders_.begin(); iter != shaders_.end(); ++iter) {
            ShaderReloadResult result = iter.Value()->BeginReload();
            if (result == ShaderReloadResult::RecompileFail) {
                failed = true;
                break;
            } else if (result == ShaderReloadResult::RecompileOk) {
                modified_shaders.PushBack(iter.Value().get());
            }
        }

        if (failed == true) {
            for (IShaderSource* shader : modified_shaders) {
                shader->DropReload();
            }
            return;
        }

        Array<IPipelineBuilder*> modified_pipelines;

        for (IPipelineBuilder* pipeline : pipelines_) {
            ShaderReloadResult result = pipeline->BeginRecreate();
            if (result == ShaderReloadResult::RecompileFail) {
                failed = true;
                break;
            } else if (result == ShaderReloadResult::RecompileOk) {
                modified_pipelines.PushBack(pipeline);
            }
        }

        if (failed) {
            for (IShaderSource* shader : modified_shaders) {
                shader->DropReload();
            }

            for (IPipelineBuilder* pipeline : modified_pipelines) {
                pipeline->DropRecreate();
            }

            return;
        }

        for (IShaderSource* shader : modified_shaders) {
            shader->CommitReload();
        }

        for (IPipelineBuilder* pipeline : modified_pipelines) {
            pipeline->CommitRecreate();
        }
    }

    //

    Optional<Array<u8>> FileShaderSource::Compile()
    {
        IDxcBlobEncoding* source_blob;
        verify_hr(Compiler::GetInstance().library_->CreateBlobFromFile(file_path_, nullptr, &source_blob));

        IDxcBlob* compiled_blob = Compiler::GetInstance().CompileShader(source_blob, file_path_, entrypoint_, profile_);
        source_blob->Release();

        if (!compiled_blob) {
            return {};
        }

        Array<u8> bytecode = Array<u8>::From(static_cast<u8*>(compiled_blob->GetBufferPointer()), compiled_blob->GetBufferSize());

        compiled_blob->Release();

        return bytecode;
    }

    Optional<u64> FileShaderSource::Preprocess()
    {
        IDxcBlobEncoding* source_blob = nullptr;
        // TODO: reading the file twice is a bit wasteful
        verify_hr(Compiler::GetInstance().library_->CreateBlobFromFile(file_path_, nullptr, &source_blob));

        IDxcBlob* preprocessed_blob = Compiler::GetInstance().PreprocessShader(source_blob, file_path_);
        source_blob->Release();

        u64 hash = HashMemory(preprocessed_blob->GetBufferPointer(), preprocessed_blob->GetBufferSize());

        preprocessed_blob->Release();
        return hash;
    }

    FileShaderSource::FileShaderSource(ShaderFileSourceDesc desc)
        : file_path_(desc.file_path)
        , entrypoint_(desc.entrypoint)
        , profile_(desc.profile)
    {
        Optional<Array<u8>> maybe_bytecode = Compile();
        plgr_assert(maybe_bytecode);
        bytecode_ = std::move(*maybe_bytecode);
        hash_ = *Preprocess();
    }

    ShaderReloadResult FileShaderSource::BeginReload()
    {
        plgr_assert(!pending_bytecode_);

        Optional<u64> hash = Preprocess();
        if (!hash) {
            return ShaderReloadResult::RecompileFail;
        }
        if (hash == hash_) {
            return ShaderReloadResult::NoChange;
        }

        pending_bytecode_ = Compile();

        if (!pending_bytecode_) {
            return ShaderReloadResult::RecompileFail;
        }

        pending_hash_ = hash;

        return ShaderReloadResult::RecompileOk;
    }

    bool FileShaderSource::IsTransitionPending()
    {
        return bool { pending_bytecode_ };
    }

    void FileShaderSource::CommitReload()
    {
        plgr_assert(pending_bytecode_ && pending_hash_);
        bytecode_ = std::move(*pending_bytecode_);
        hash_ = *pending_hash_;
        pending_bytecode_ = NullOpt;
        pending_hash_ = NullOpt;
    }

    void FileShaderSource::DropReload()
    {
        plgr_assert(pending_bytecode_ && pending_hash_);
        pending_bytecode_ = NullOpt;
        pending_hash_ = NullOpt;
    }

    D3D12_SHADER_BYTECODE FileShaderSource::GetBytecode()
    {
        if (pending_bytecode_) {
            return { .pShaderBytecode = pending_bytecode_->Data(), .BytecodeLength = As<u64>(pending_bytecode_->Size()) };
        }

        return { .pShaderBytecode = bytecode_.Data(), .BytecodeLength = static_cast<u64>(bytecode_.Size()) };
    }

    StaticShaderSource::StaticShaderSource(ShaderStaticSourceDesc desc)
    {
        IDxcBlob* source_blob = Wrap(desc.source, strlen(desc.source));
        IDxcBlob* compiled_blob = Compiler::GetInstance().CompileShader(source_blob, L"[static_shader]", desc.entrypoint, desc.profile);
        bytecode_ = Array<u8>::From(static_cast<u8*>(compiled_blob->GetBufferPointer()), compiled_blob->GetBufferSize());
        source_blob->Release();
        compiled_blob->Release();
    }

    bool StaticShaderSource::IsTransitionPending()
    {
        return false;
    }

    ShaderReloadResult StaticShaderSource::BeginReload()
    {
        return ShaderReloadResult::NoChange;
    }

    void StaticShaderSource::CommitReload()
    {
        DEBUG_UNREACHABLE(default_module {});
    }

    void StaticShaderSource::DropReload()
    {
        DEBUG_UNREACHABLE(default_module {});
    }

    D3D12_SHADER_BYTECODE StaticShaderSource::GetBytecode()
    {
        return { .pShaderBytecode = bytecode_.Data(), .BytecodeLength = static_cast<u64>(bytecode_.Size()) };
    }

    IPipelineBuilder::IPipelineBuilder()
    {
        ShaderCache::GetInstance().Register(this);
    }

    IPipelineBuilder::~IPipelineBuilder()
    {
        ShaderCache::GetInstance().Deregister(this);
    }

    void ReloadShaders()
    {
        ShaderCache::GetInstance().Reload();
    }

}

}