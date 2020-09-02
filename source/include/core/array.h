#pragma once

#include "containers_shared.h"
#include "Core.h"
#include <string.h>

namespace Playground {
template <typename T>
struct Array {
    i64 size_ = 0;
    i64 max_size_ = 0;
    T* data_ = nullptr;

    struct Iterator {
        Array* const array_;
        i64 index_ = 0;

        Iterator(Array* const array, i64 index)
            : array_(array)
            , index_(index)
        {
        }

        Iterator operator++(int)
        {
            index_++;
            return *this;
        }

        Iterator& operator++()
        {
            index_++;
            return *this;
        }

        bool operator==(Iterator other) const
        {
            return array_ == other.array_ && index_ == other.index_;
        }

        bool operator!=(Iterator other) const
        {
            return !((*this) == other);
        }

        T& operator*() const
        {
            return array_->At(index_);
        }
    };

    struct ConstIterator {
        const Array* const array_;
        i64 index_ = 0;

        ConstIterator(const Array* const array, i64 index)
            : array_(array)
            , index_(index)
        {
        }

        ConstIterator operator++(int)
        {
            index_++;
            return *this;
        }

        ConstIterator& operator++()
        {
            index_++;
            return *this;
        }

        bool operator==(ConstIterator other) const
        {
            return array_ == other.array_ && index_ == other.index_;
        }

        bool operator!=(ConstIterator other) const
        {
            return !((*this) == other);
        }

        const T& operator*() const
        {
            return array_->At(index_);
        }
    };

    Array() = default;

    ~Array()
    {
        Release();
    }

    Array(Array const& rhs)
    {
        ResizeUninitialised(rhs.Size());
        memcpy(data_, rhs.data_, sizeof(T) * Size());
    }

    Array& operator=(Array const& rhs)
    {
        Release();
        ResizeUninitialised(rhs.Size());
        memcpy(data_, rhs.data_, sizeof(T) * Size());
        return *this;
    }

    Array(Array&& rhs)
    {
        data_ = rhs.data_;
        size_ = rhs.size_;
        max_size_ = rhs.max_size_;

        rhs.data_ = nullptr;
        rhs.max_size_ = rhs.size_ = 0;
    }

    Array& operator=(Array&& rhs)
    {
        Release();
        data_ = rhs.data_;
        size_ = rhs.size_;
        max_size_ = rhs.max_size_;

        rhs.data_ = nullptr;
        rhs.max_size_ = rhs.size_ = 0;

        return *this;
    }

    Iterator begin()
    {
        return Iterator { this, 0 };
    }

    Iterator end()
    {
        return Iterator { this, size_ };
    }

    ConstIterator cbegin() const
    {
        return ConstIterator { this, 0 };
    }

    ConstIterator cend() const
    {
        return ConstIterator { this, size_ };
    }

    void Reserve(i64 min_size)
    {
        i64 max_size = max_size_;

        if (max_size == 0) {
            max_size = min_size;
        }

        while (max_size < min_size) {
            if (max_size < 1024) {
                max_size *= 2;
            } else {
                max_size = max_size * 3 / 2;
            }
        }

        if (max_size != max_size_) {
            max_size_ = max_size;
            data_ = static_cast<T*>(realloc(data_, sizeof(T) * max_size_));
        }
    }

    void _Resize(i64 size, bool initialise)
    {
        plgr_assert(size >= 0);
        Reserve(size);

        if (initialise) {
            for (i64 i = size_; i < size; i++) {
                new (data_ + i) T();
            }
        }

        if constexpr (!std::is_trivial_v<T>) {
            for (i64 i = size_ - 1; i >= size; i--) {
                (data_ + i)->T::~T();
            }
        }

        size_ = size;
    }

    void ExpandToIndex(i64 index)
    {
        if(Size() <= index) 
        {
            Resize(index + 1);
        }
    }

    void ResizeUninitialised(i64 size)
    {
        _Resize(size, false);
    }

    void Resize(i64 size)
    {
        _Resize(size, true);
    }

    void Clear()
    {
        Resize(0);
    }

    i64 Size() const
    {
        return size_;
    }

    static Array<T> From(T const* src, i64 num)
    {
        Array<T> result;
        result.Append(src, num);
        return result;
    }

    void Append(T const* src, i64 num)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        Reserve(size_ + num);
        memcpy(data_ + size_, src, num * sizeof(T));
        size_ += num;
    }

    void AppendZeroed(i64 num)
    {
        Resize(size_ + num);
    }

    void PopNum(i64 num)
    {
        Resize(size_ - num);
    }

    void Shrink()
    {
        if (max_size_ == size_) {
            return;
        }

        if (size_ == 0) {
            Release();
            return;
        }

        data_ = static_cast<T*>(realloc(data_, size_ * sizeof(T)));
        max_size_ = size_;
    }

    void Release()
    {
        Resize(0);
        free(data_);
        data_ = nullptr;
        max_size_ = 0;
    }

    const T* Data() const
    {
        return data_;
    }

    T* Data()
    {
        return data_;
    }

    const T& At(i64 index) const
    {
        //static_assert(std::is_trivially_copyable_v<T>);
        DEBUG_ASSERT(index == Clamp(index, 0ll, Size() - 1), containers_module {});
        return data_[index];
    }

    T& At(i64 index)
    {
        //static_assert(std::is_trivially_copyable_v<T>);
        DEBUG_ASSERT(index == Clamp(index, 0ll, Size() - 1), containers_module {});
        return data_[index];
    }

    const T& operator[](i64 index) const
    {
        DEBUG_ASSERT(index == Clamp(index, 0ll, Size() - 1), containers_module {});
        return data_[index];
    }

    T& operator[](i64 index)
    {
        DEBUG_ASSERT(index == Clamp(index, 0ll, Size() - 1), containers_module {});
        return data_[index];
    }

    T PopBack()
    {
        DEBUG_ASSERT(size_ > 0, containers_module {});
        size_ -= 1;
        if constexpr (std::is_trivially_copyable_v<T>) {
            return data_[size_];
        }

        static_assert(std::is_move_constructible_v<T>);
        return std::move(data_[size_]);
    }

    void PushBackUninitialised()
    {
        ResizeUninitialised(size_ + 1);
    }

    void PushBack(T t)
    {
        static_assert(std::is_trivially_copyable_v<T>);

        ResizeUninitialised(size_ + 1);
        data_[size_ - 1] = t;
    }

    void PushBackRvalueRef(T&& t)
    {
        static_assert(std::is_move_assignable_v<T>);

        Resize(size_ + 1);
        data_[size_ - 1] = std::move(t);
    }

    T RemoveAt(i64 index)
    {
        if constexpr (std::is_trivially_copyable_v<T>) {
            T obj = At(index);

            memmove(data_ + index, data_ + index + 1, (size_ - index - 1) * sizeof(T));
            size_--;

            return obj;
        }

        static_assert(std::is_move_assignable_v<T>);
        T obj = std::move(data_[index]);

        for (i64 i = index; i < size_ - 1; i++) {
            data_[i] = std::move(data_[i + 1]);
        }

        size_--;
        return obj;
    }

    bool RemoveAndSwapWithLast(T const & value) {
        static_assert(std::is_trivially_copyable_v<T>);

        for (i64 i = 0; i < size_; i++) {
            if (data_[i] == value) {
                RemoveAtAndSwapWithLast(i);
                return true;
            }
        }

        return false;
    }

    void RemoveAtAndSwapWithLast(i64 index)
    {
        static_assert(std::is_trivially_copyable_v<T>);

        if (index != size_ - 1) {
            data_[index] = data_[size_ - 1];
        }

        size_--;
    }

    Optional<i64> Find(const T& item) const
    {
        for (i64 i = 0, N = Size(); i < N; i++) {
            if (data_[i] == item) {
                return i;
            }
        }
        return NullOpt;
    }

    bool Contains(const T& item) const
    {
        return bool { Find(item) };
    }

    T& First()
    {
        return (*this)[0];
    }

    T& Last()
    {
        return (*this)[Size() - 1];
    }
};

}