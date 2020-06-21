#pragma once

struct IDxcBlob;

namespace Gfx {
	using ShaderBlob = IDxcBlob;

	ShaderBlob* CompileShaderFromFile(
		const wchar_t* source_path,
		const wchar_t* file_name,
		const wchar_t* entrypoint,
		const wchar_t* target);
}