#pragma once

#include "box.h"
#include "types.h"

namespace Random {
struct Generator : private Core::Pinned<Generator> {
    void* state_ = nullptr;

    Generator(u32 seed = 0);
    ~Generator();

    u32 U32Uniform();
    i32 I32UniformInRange(i32 from, i32 to);
    f32 F32Uniform(); // [0,1)
    f32 F32UniformInRange(f32 from, f32 to);
};
}