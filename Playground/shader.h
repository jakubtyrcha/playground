#pragma once
#include "Array.h"
#include "Box.h"
#include "Gfx.h"

struct D3D12_SHADER_BYTECODE;

// TODO: move to a separate file
namespace Core {
struct String {
    Containers::Array<wchar_t> data_;

    String();
    String(const wchar_t*);
    operator const wchar_t*() const;
};
}

namespace Gfx {
enum class ShaderReloadResult {
    NoChange,
    RecompileOk,
    RecompileFail,
};

struct IShaderSource {
    u64 identifier_ = 0;

    virtual bool IsTransitionPending() = 0;
    virtual ShaderReloadResult BeginReload() = 0;
    virtual void CommitReload() = 0;
    virtual void DropReload() = 0;
    virtual D3D12_SHADER_BYTECODE GetBytecode() = 0;
};

struct ShaderStaticSourceDesc {
    const char* source;
    const wchar_t* entrypoint;
    const wchar_t* profile;
};

struct ShaderFileSourceDesc {
    const wchar_t* file_path;
    const wchar_t* entrypoint;
    const wchar_t* profile;
};

struct StaticShaderSource : public IShaderSource {
    Containers::Array<u8> bytecode_;

    StaticShaderSource(ShaderStaticSourceDesc desc);

    bool IsTransitionPending() override;
    ShaderReloadResult BeginReload() override;
    void CommitReload() override;
    void DropReload() override;

    D3D12_SHADER_BYTECODE GetBytecode() override;
};

struct FileShaderSource : public IShaderSource {
    Core::String file_path_;
    Core::String entrypoint_;
    Core::String profile_;

    u64 hash_;
    Containers::Array<u8> bytecode_;
    Core::Optional<u64> pending_hash_;
    Core::Optional<Containers::Array<u8>> pending_bytecode_;

    FileShaderSource(ShaderFileSourceDesc desc);

    Core::Optional<u64> Preprocess();
    Core::Optional<Containers::Array<u8>> Compile();

    bool IsTransitionPending() override;
    ShaderReloadResult BeginReload() override;
    void CommitReload() override;
    void DropReload() override;

    D3D12_SHADER_BYTECODE GetBytecode() override;
};

struct IPipelineBuilder {
    Core::Box<Pipeline> pipeline_;
    Containers::Array<IShaderSource*> shaders_;

    Core::Optional<Core::Box<Pipeline>> pending_pipeline_;

    IPipelineBuilder();
    virtual ~IPipelineBuilder();
    virtual Core::Optional<Core::Box<Pipeline>> Build() = 0;

    ShaderReloadResult BeginRecreate();
    void CommitRecreate();
    void DropRecreate();

    ID3D12PipelineState* GetPSO();

    IShaderSource* GetShaderFromStaticSource(ShaderStaticSourceDesc);
    IShaderSource* GetShaderFromShaderFileSource(ShaderFileSourceDesc);
};

void ReloadShaders();
}