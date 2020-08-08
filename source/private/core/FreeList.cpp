#include "pch.h"
#include "FreeList.h"

namespace Playground {

i32 FreeList::Allocate()
{
    if (!freelist_.Size()) {
        return next_++;
    }

    return freelist_.PopBack();
}

void FreeList::Free(i32 index)
{
    freelist_.PushBack(index);
}
}