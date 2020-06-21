#include "pch.h"
#include <dxcapi.h>
#include <dxil/inc/dxcapi.use.h>
#include "gfx.h"

#include <magnum/CorradeOptional.h>

template<typename T>
using Optional = Corrade::Containers::Optional<T>;

namespace Gfx {
	IDxcBlob* CompileShaderFromFile(
		const wchar_t* source_path,
		const wchar_t* file_name,
		const wchar_t* entrypoint,
		const wchar_t* target)
	{
		static dxc::DxcDllSupport dxc_dll_support;

		dxc_dll_support.Initialize();

		IDxcCompiler* compiler;
		IDxcOperationResult* operation_result;
		verify_hr(dxc_dll_support.CreateInstance(CLSID_DxcCompiler, &compiler));

		IDxcLibrary* library;
		IDxcBlobEncoding* source;
		verify_hr(dxc_dll_support.CreateInstance(CLSID_DxcLibrary, &library));
		verify_hr(library->CreateBlobFromFile(source_path, nullptr, &source));

		const wchar_t* arguments[] = { L"" };

		IDxcIncludeHandler* include_handler = nullptr;
		verify_hr(library->CreateIncludeHandler(&include_handler));

		HRESULT hr = compiler->Compile(source, file_name, entrypoint, target, arguments, _countof(arguments),
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
}