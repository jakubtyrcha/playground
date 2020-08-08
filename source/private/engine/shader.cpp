#include "pch.h"
#include "Shader.h"
#include "Gfx.h"

namespace Playground {
namespace Gfx {
    Shader::~Shader() {}

    void Shader::Init(Device* device)
    {
        device_ = device;
    }
}
}