#pragma once

#include <dxcapi.h>

namespace Core {
	struct String {
		Containers::Array<wchar_t> data_;

		String(const wchar_t *);
	};
}

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

	enum class ShaderReloadResult {
		NoChange,
		RecompileOk,
		RecompileFail,
	};

	struct IShaderSource {
		virtual ShaderReloadResult Reload() = 0;
		virtual D3D12_SHADER_BYTECODE GetBytecode() = 0;
	};

	struct FileShaderSource : public IShaderSource {
		Core::String file_path_;
		Core::String entrypoint_;
		Core::String profile_;

		ShaderReloadResult Reload() override;
		D3D12_SHADER_BYTECODE GetBytecode() override;
	};

	struct IPipelineConsumer;

	struct ShaderCache {
		Array<Box<IShaderSource>> shaders_;
		Array<IPipelineConsumer*> pipeline_consumers_;

		struct ShaderFileSourceDesc {
			const wchar_t* file_path;
			const wchar_t* entrypoint;
			const wchar_t* profile;
		};

		IShaderSource* GetShaderSourceFromFile(ShaderFileSourceDesc);
		void RegisterConsumer(IPipelineConsumer*);
		void UnregisterConsumer(IPipelineConsumer*);
	};
	
	struct IPipelineConsumer {
		Array<IShaderSource*> shaders_;

		virtual ShaderReloadResult CreatePipelines() = 0;
	};
}