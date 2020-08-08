#pragma once

#include "array.h"

namespace Playground {

struct FreeList {
    i32 next_ = 0;
    Array<i32> freelist_;

    i32 Allocate();
    void Free(i32);
};

}