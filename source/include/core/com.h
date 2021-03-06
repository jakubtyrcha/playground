#pragma once

namespace Playground {
namespace Com {
    template <typename T>
    struct Box {
        T* ptr_ = nullptr;

        Box() = default;

        Box(Box const&) = delete;
        Box& operator=(Box const&) = delete;

        Box(Box&& other)
        {
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }

        Box& operator=(Box&& other)
        {
            Release();
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
            return *this;
        }

        ~Box()
        {
            Release();
        }

        void Release()
        {
            if (ptr_) {
                ptr_->Release();
                ptr_ = nullptr;
            }
        }

        T** InitAddress()
        {
            return &ptr_;
        }

        T* operator->()
        {
            return ptr_;
        }

        T* Get()
        {
            return ptr_;
        }

        T* operator*()
        {
            return Get();
        }

        T* Get() const
        {
            return ptr_;
        }

        T* operator*() const
        {
            return Get();
        }
    };
}
}