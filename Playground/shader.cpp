#include "pch.h"
#include <dxcapi.h>
#include <dxil/inc/dxcapi.use.h>
#include "gfx.h"
#include "shader.h"
#include "hash.h"

template<typename T>
using Optional = Corrade::Containers::Optional<T>;

namespace Gfx {
	struct Compiler {
		dxc::DxcDllSupport dxc_dll_support_;
		IDxcCompiler* compiler_ = nullptr;
		IDxcLibrary* library_ = nullptr;

		Compiler() {
			dxc_dll_support_.Initialize();

			verify_hr(dxc_dll_support_.CreateInstance(CLSID_DxcCompiler, &compiler_));
			verify_hr(dxc_dll_support_.CreateInstance(CLSID_DxcLibrary, &library_));
		}

		~Compiler() {
		}

		static Compiler& GetInstance() {
			static Compiler instance;
			return instance;
		}

		IDxcBlob* CompileShader(
			IDxcBlob* source,
			IDxcIncludeHandler* include_handler,
			const wchar_t* file_name,
			const wchar_t* entrypoint,
			const wchar_t* target
		)
		{
			IDxcOperationResult* operation_result;

			HRESULT hr = compiler_->Compile(source, file_name, entrypoint, target, nullptr, 0,
				nullptr, 0, include_handler, &operation_result);
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
				return bytecode;
			}
			else {
				ID3DBlob* error_messages;
				verify_hr(operation_result->GetErrorBuffer((IDxcBlobEncoding**)&error_messages));
				fmt::print("Failed to compile shader: {}", (const char*)error_messages->GetBufferPointer());
				error_messages->Release();

				return nullptr;
			}

			DEBUG_UNREACHABLE(default_module{});
			return nullptr;
		}
	};

	IDxcBlob* Wrap(const char* ptr, i64 len);

	struct WrappedBlob : public IDxcBlob {
		const char* ptr_ = nullptr;
		u64 size_ = 0;
		u32 refs_ = 1;

		WrappedBlob(const char* ptr, u64 size) : ptr_(ptr), size_(size) {
		}

		HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override {
			return E_NOTIMPL;
		}

		ULONG __stdcall AddRef(void) override {
			return refs_++;
		}

		ULONG __stdcall Release(void) override {
			u32 result = --refs_;
			if (result == 0) {
				delete this;
			}
			return result;
		}

		LPVOID __stdcall GetBufferPointer(void) override {
			return const_cast<void*>(reinterpret_cast<const void*>(ptr_));
		}

		virtual SIZE_T __stdcall GetBufferSize(void) override {
			return size_;
		}
	};

	IDxcBlob* Wrap(const char* ptr, i64 len) {
		return new WrappedBlob(ptr, len);
	}

	//

	ShaderReloadResult IPipelineBuilder::BeginRecreate() {
		return ShaderReloadResult::RecompileFail;
	}

	void IPipelineBuilder::CommitRecreate() {

	}

	void IPipelineBuilder::DropRecreate() {
	}

	ID3D12PipelineState * IPipelineBuilder::GetPSO() {
		if(!pipeline_) {
			pipeline_ = Build();
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

		static ShaderCache& GetInstance();
	};

	IShaderSource* IPipelineBuilder::GetShaderFromStaticSource(ShaderStaticSourceDesc desc) {
		return ShaderCache::GetInstance().GetShaderFromStaticSource(desc);
	}

	IShaderSource* IPipelineBuilder::GetShaderFromShaderFileSource(ShaderFileSourceDesc desc) {
		return ShaderCache::GetInstance().GetShaderFromShaderFileSource(desc);
	}

	ShaderCache& ShaderCache::GetInstance() {
		static ShaderCache instance;
		return instance;
	}

	IShaderSource* ShaderCache::GetShaderFromStaticSource(ShaderStaticSourceDesc desc) {
		u64 hash = 0;
		hash = Hash::HashMemory(desc.source, strlen(desc.source));
		hash = Hash::HashMemoryWithSeed(desc.entrypoint, wcslen(desc.entrypoint) * sizeof(wchar_t), hash);
		hash = Hash::HashMemoryWithSeed(desc.profile, wcslen(desc.profile) * sizeof(wchar_t), hash);

		if(Optional<Box<IShaderSource>*> shader = shaders_.Find(hash); shader) {
			return (*shader)->get();
		}

		Box<IShaderSource> shader = MakeBox<StaticShaderSource>(desc);
		IShaderSource * ptr = shader.get();
		shaders_.InsertRvalueRef(hash, std::move(shader));

		return ptr;
	}

	IShaderSource* ShaderCache::GetShaderFromShaderFileSource(ShaderFileSourceDesc desc) {
		u64 hash = 0;
		hash = Hash::HashMemory(desc.file_path, wcslen(desc.file_path) * sizeof(wchar_t));
		hash = Hash::HashMemoryWithSeed(desc.entrypoint, wcslen(desc.entrypoint) * sizeof(wchar_t), hash);
		hash = Hash::HashMemoryWithSeed(desc.profile, wcslen(desc.profile) * sizeof(wchar_t), hash);

		if(Optional<Box<IShaderSource>*> shader = shaders_.Find(hash); shader) {
			return (*shader)->get();
		}

		Box<IShaderSource> shader = MakeBox<FileShaderSource>(desc);
		IShaderSource * ptr = shader.get();
		shaders_.InsertRvalueRef(hash, std::move(shader));

		return ptr;
	}

	void ShaderCache::Register(IPipelineBuilder* pb) {
		pipelines_.PushBack(pb);		
	}

	void ShaderCache::Deregister(IPipelineBuilder* pb) {
		for(i32 i=0; i<pipelines_.Size(); i++) {
			if(pipelines_[i] == pb) {
				pipelines_.RemoveAtAndSwapWithLast(i);
				return;
			}
		}
	}

	//

	FileShaderSource::FileShaderSource(ShaderFileSourceDesc desc) :
		file_path_(desc.file_path),
		entrypoint_(desc.entrypoint),
		profile_(desc.profile)
	{
		BeginReload();
		bytecode_ = std::move(*pending_bytecode_);
		pending_bytecode_ = {};
	}

	ShaderReloadResult FileShaderSource::BeginReload() {
		assert(!pending_bytecode_);

		IDxcBlobEncoding* source_blob;
		verify_hr(Compiler::GetInstance().library_->CreateBlobFromFile(file_path_, nullptr, &source_blob));

		// TODO: hash preprocessed file instead
		u64 content_hash = Hash::HashMemory(source_blob->GetBufferPointer(), source_blob->GetBufferSize());

		if(content_hash_ && content_hash_ == content_hash) {
			return ShaderReloadResult::NoChange;
		}

		static IDxcIncludeHandler* include_handler;
		if (!include_handler) {
			verify_hr(Compiler::GetInstance().library_->CreateIncludeHandler(&include_handler));
		}

		IDxcBlob * compiled_blob = Compiler::GetInstance().CompileShader(source_blob, include_handler, file_path_, entrypoint_, profile_);
		source_blob->Release();

		if(!compiled_blob) {
			return ShaderReloadResult::RecompileFail;
		}

		pending_bytecode_ = Array<u8>::From(static_cast<u8*>(compiled_blob->GetBufferPointer()), compiled_blob->GetBufferSize());
		compiled_blob->Release();

		return ShaderReloadResult::RecompileOk;
	}

	void FileShaderSource::CommitReload() {
		DEBUG_UNREACHABLE(default_module{});
	}

	void FileShaderSource::DropReload() {
		DEBUG_UNREACHABLE(default_module{});
	}

	D3D12_SHADER_BYTECODE FileShaderSource::GetBytecode() {
		return { .pShaderBytecode = bytecode_.Data(), .BytecodeLength = static_cast<u64>(bytecode_.Size()) };
	}

	StaticShaderSource::StaticShaderSource(ShaderStaticSourceDesc desc) {
		IDxcBlob * source_blob = Wrap(desc.source, strlen(desc.source));

		IDxcBlob * compiled_blob = Compiler::GetInstance().CompileShader(source_blob, nullptr, L"[static_shader]", desc.entrypoint, desc.profile);

		bytecode_ = Array<u8>::From(static_cast<u8*>(compiled_blob->GetBufferPointer()), compiled_blob->GetBufferSize());

		source_blob->Release();
		compiled_blob->Release();
	}

	ShaderReloadResult StaticShaderSource::BeginReload() {
		return ShaderReloadResult::NoChange;
	}

	void  StaticShaderSource::CommitReload() {
		DEBUG_UNREACHABLE(default_module{});
	}

	void StaticShaderSource::DropReload() {
		DEBUG_UNREACHABLE(default_module{});
	}

	D3D12_SHADER_BYTECODE StaticShaderSource::GetBytecode() {
		return { .pShaderBytecode = bytecode_.Data(), .BytecodeLength = static_cast<u64>(bytecode_.Size()) };
	}

	IPipelineBuilder::IPipelineBuilder() {
		ShaderCache::GetInstance().Register(this);
	}

	IPipelineBuilder::~IPipelineBuilder() {
		ShaderCache::GetInstance().Deregister(this);
	}
}

namespace Core {
	String::String() {
		data_.Resize(1);
	}

	String::String(const wchar_t * str) {
		i64 len = wcslen(str);
		data_.Reserve(len + 1);
		data_.Append(str, len);
		data_.PushBack(0);
	}

	String::operator const wchar_t *() const {
		return data_.Data();
	}
}