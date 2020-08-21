#pragma once
#include "Types.h"
#include "Core.h"
#include "FreeList.h"

namespace Playground {

struct Handle32 {
    static constexpr i32 INDEX_BITS = 24; // 16 mln 
    static constexpr i32 GENERATION_BITS = 8;

    static constexpr i32 MAX_INDEX = (1 << INDEX_BITS) - 1;
    static constexpr i32 MAX_GENERATION = (1 << GENERATION_BITS) - 1;

    u32 value_ = 0;

    bool operator()()const;

    bool IsNull() const;
    i32 GetIndex() const;
    i32 GetGeneration() const;

    static Handle32 Make(i32 index, i32 generation);
};

template<typename T, typename _Handle>
struct DenseArray
{
    using Handle = _Handle;

    Array<T> data_;
    Array<i8> generation_;
    FreeList freelist_;
    Array<i32> indirection_;
    Array<i32> rev_indirection_;

    Handle Insert(T v)
    {
        i32 indirect_index = freelist_.Allocate();
        indirection_.Reserve(indirect_index + 1);
        rev_indirection_.Reserve(indirect_index + 1);

        if (indirection_.Size() == indirect_index) {
            indirection_.PushBack(-1);
            rev_indirection_.PushBack(-1);
        }

        plgr_assert(indirection_[indirect_index] == -1);
        i32 flat_index = As<i32>(data_.Size());
        indirection_[indirect_index] = As<i32>(data_.Size());
        plgr_assert(rev_indirection_[flat_index] == -1);
        rev_indirection_[flat_index] = indirect_index;

        data_.PushBackRvalueRef(v);
        generation_.ExpandToIndex(flat_index);
        i8 slot_generation = Max(As<i8>(1), generation_[flat_index]);
        generation_[flat_index] = slot_generation;

        return Handle::Make(indirect_index, slot_generation);
    }

    void Remove(Handle h)
    {
        i32 flat_index = indirection_[h.GetIndex()];
        plgr_assert(h.GetGeneration() == generation_[flat_index]);
        i32 last_flat_index = As<i32>(data_.Size() - 1);
        plgr_assert(last_flat_index >= 0);
        data_.RemoveAtAndSwapWithLast(flat_index);
        generation_[flat_index] = generation_[last_flat_index];
        generation_[last_flat_index] = (generation_[last_flat_index] + 1) % (Handle::MAX_GENERATION + 1);
        plgr_assert(rev_indirection_[last_flat_index] >= 0);
        plgr_assert(indirection_[rev_indirection_[last_flat_index]] >= 0);
        i32 last_indirect_index = rev_indirection_[last_flat_index];
        indirection_[last_indirect_index] = flat_index;
        indirection_[h.GetIndex()] = -1;
        rev_indirection_[flat_index] = last_indirect_index;
        rev_indirection_[last_flat_index] = -1;
        freelist_.Free(h.GetIndex());
    }

    T& operator[](Handle h)
    {
        i32 flat_index = indirection_[h.GetIndex()];
        plgr_assert(h.GetGeneration() == generation_[flat_index]);
        return data_[flat_index];
    }

    i64 Size() const
    {
        return data_.Size();
    }
};

}