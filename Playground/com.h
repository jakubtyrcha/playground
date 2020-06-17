#pragma once

namespace Com
{
	template<typename T>
	struct UniqueComPtr
	{
		T* ptr_ = nullptr;

		UniqueComPtr() = default;

		UniqueComPtr(UniqueComPtr const&) = delete;
		UniqueComPtr& operator=(UniqueComPtr const&) = delete;

		UniqueComPtr(UniqueComPtr&& other)
		{
			ptr_ = other.ptr_;
			other.ptr_ = nullptr;
		}

		UniqueComPtr& operator=(UniqueComPtr&& other)
		{
			Release();
			ptr_ = other.ptr_;
			other.ptr_ = nullptr;
			return *this;
		}

		~UniqueComPtr()
		{
			Release();
		}

		void Release()
		{
			if (ptr_)
			{
				ptr_->Release();
				ptr_ = nullptr;
			}
		}

		T** PtrAddress()
		{
			return &ptr_;
		}

		T* operator->()
		{
			return ptr_;
		}
	};
}