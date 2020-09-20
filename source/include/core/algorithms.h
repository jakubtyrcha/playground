#pragma once

#include "types.h"

namespace Playground {
template <typename T>
i64 LowerBound(const T* array, i64 size, T value)
{
    i64 l = 0;
    i64 r = size;
    i64 c = r - l;
    while (c > 0) {
        i64 step = c / 2;
        if (array[l + step] < value) {
            l = l + step + 1;
            c -= step + 1;
        } else {
            c = step;
        }
    }

    return l;
}

f32 Frac(f32 x);

template<typename T, typename F>
void RemoveIf(T& container, F&& condition_functor) {
    for (i64 index = 0; index < container.Size(); ) {
        if (condition_functor(container.At(index))) {
            container.RemoveAt(index);
        }
        else {
            index++;
        }
    }
}
}