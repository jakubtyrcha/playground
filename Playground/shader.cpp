#include "pch.h"
#include <dxcapi.h>
#include <dxil/inc/dxcapi.use.h>
#include "gfx.h"

#include <magnum/CorradeOptional.h>

template<typename T>
using Optional = Corrade::Containers::Optional<T>;

namespace Gfx {
	dxc::DxcDllSupport& GetDxcDllSupport() {
		static dxc::DxcDllSupport dxc_dll_support;
		static bool once = false;
		if (!once) { // TODO: replace with sth fancier
			dxc_dll_support.Initialize();
		}
		return dxc_dll_support;
	}

	IDxcBlob* CompileShader(
		IDxcBlob* source,
		IDxcIncludeHandler* include_handler,
		const wchar_t* file_name,
		const wchar_t* entrypoint,
		const wchar_t* target
	)
	{
		static IDxcCompiler* compiler;
		IDxcOperationResult* operation_result;
		if (!compiler) {
			verify_hr(GetDxcDllSupport().CreateInstance(CLSID_DxcCompiler, &compiler));
		}

		HRESULT hr = compiler->Compile(source, file_name, entrypoint, target, nullptr, 0,
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

	IDxcBlob* CompileShaderFromFile(
		const wchar_t* source_path,
		const wchar_t* file_name,
		const wchar_t* entrypoint,
		const wchar_t* target)
	{

		static IDxcLibrary* library;
		IDxcBlobEncoding* source;
		if (!library) {
			verify_hr(GetDxcDllSupport().CreateInstance(CLSID_DxcLibrary, &library));
		}
		verify_hr(library->CreateBlobFromFile(source_path, nullptr, &source));

		static IDxcIncludeHandler* include_handler;
		if (!include_handler) {
			verify_hr(library->CreateIncludeHandler(&include_handler));
		}

		return CompileShader(source, include_handler, source_path, entrypoint, target);
	}

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
}