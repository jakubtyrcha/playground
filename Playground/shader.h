#pragma once

#include <dxcapi.h>

namespace Gfx {
	using ShaderBlob = IDxcBlob;

	ShaderBlob* CompileShaderFromFile(
		const wchar_t* source_path,
		const wchar_t* file_name,
		const wchar_t* entrypoint,
		const wchar_t* target);

	ShaderBlob* CompileShader(
		ShaderBlob* source,
		IDxcIncludeHandler* include_handler,
		const wchar_t* file_name,
		const wchar_t* entrypoint,
		const wchar_t* target
	);

	ShaderBlob* Wrap(const char* ptr, i64 len);
}