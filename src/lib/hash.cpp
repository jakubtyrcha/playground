#include "pch.h"

#include "hash.h"
#include <xxhash/xxh3.h>

namespace Playground {
namespace Hash {
    template <>
    u64 HashValue(i32 x) { return x; }
    template <>
    u64 HashValue(i64 x) { return x; }
    template <>
    u64 HashValue(u32 x) { return x; }
    template <>
    u64 HashValue(u64 x) { return x; }
}

u64 HashMemory(const void* ptr, i64 size)
{
    return XXH3_64bits(ptr, size);
}

u64 HashMemoryWithSeed(const void* ptr, i64 size, u64 seed)
{
    return XXH3_64bits_withSeed(ptr, size, seed);
}
}