#pragma once

#include "array.h"
#include "containers_shared.h"

namespace Containers {
struct Bitarray {
    Array<i64> data_;
    i64 size_ = 0;

    Bitarray() = default;

    void Resize(i64 size);
    void SetBit(i64 index, bool v);
    bool GetBit(i64 index) const;
    bool AnyBitSet() const;
    i64 GetNextBitSet(i64 start_index) const;
    void ClearAll();
    void Shrink();
};
}