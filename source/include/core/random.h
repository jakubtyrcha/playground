#pragma once

#include "box.h"
#include "types.h"

namespace Playground {
struct Rng : private Pinned<Rng> {
    void* state_ = nullptr;

    Rng(u32 seed = 0);
    ~Rng();

    u32 U32Uniform();
    i32 I32UniformInRange(i32 from, i32 to);
    f32 F32Uniform(); // [0,1)
    f32 F32UniformInRange(f32 from, f32 to);
};
}