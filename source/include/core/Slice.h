#pragma once
#include "Types.h"
#include "Core.h"

namespace Playground {

template <typename T>
struct Slice {
    T* data;
    i64 num;

    T& operator[](i64 index)
    {
        plgr_assert(index == Clamp(index, 0ll, num - 1));
        return data[index];
    }
};

}