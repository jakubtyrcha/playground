#pragma once
#include "Array.h"
#include "Box.h"
#include "Gfx.h"
#include "ShaderSource.h"

namespace Playground {
namespace Gfx {
    struct Shader;
    struct Device;

	template <typename TShader>
    struct ShaderPipeline : public IPipelineBuilder {
        Shader* owner_ = nullptr;

        ShaderPipeline(TShader* render_component)
            : owner_(render_component)
        {
        }
    };

    struct Shader {
        Device* device_ = nullptr;
        Box<IPipelineBuilder> pipeline_;

        virtual ~Shader();
        virtual void Init(Device* device);
    };

}
}