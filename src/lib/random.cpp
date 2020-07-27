#include "pch.h"

#include "random.h"
#include <limits>
#include <mersennetwister/mersenne-twister.h>

namespace Random {
Generator::Generator(u32 seed)
{
    mt_init(reinterpret_cast<MTState**>(&state_), seed);
}

Generator::~Generator()
{
    mt_destroy(static_cast<MTState*>(state_));
    state_ = nullptr;
}

u32 Generator::U32Uniform()
{
    return mt_rand_u32(static_cast<MTState*>(state_));
}

i32 Generator::I32UniformInRange(i32 from, i32 to)
{
    // todo: not uniform, but pretty close if the span is relatively small
    return from + U32Uniform() % (to - from);
}

f32 Generator::F32Uniform()
{
    constexpr f32 epsilon = FLT_EPSILON;
    constexpr u32 resolution = static_cast<u32>(1.f / epsilon);
    constexpr u32 buckets = UINT32_MAX / resolution;
    constexpr u32 max_value = resolution * buckets - 1;

    u32 r;
    while (true) {
        r = U32Uniform();
        if (r <= max_value) {
            break;
        }
    }

    return (r % resolution) * epsilon;
}

f32 Generator::F32UniformInRange(f32 from, f32 to)
{
    return to + (from - to) * F32Uniform();
}
}