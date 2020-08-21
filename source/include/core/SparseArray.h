#pragma once

#include "Array.h"

namespace Playground {

template<typename T>
struct SparseArray {
    i32 next_ = 0;
    Array<i32> free_slots_;
    Array<T> data_;

    i32 Allocate()
    {
        if(free_slots_.Size()) {
            return free_slots_.PopBack();
        }

        return next_++;
    }

    void Free(i32 slot)
    {
        free_slots_.PushBack(slot);
    }

    T& operator[](i32 index) {
        return data_[index];
    }
};

}