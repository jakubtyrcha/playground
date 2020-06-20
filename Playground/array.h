#pragma once

#include "containers_shared.h"

namespace Containers
{
	template<typename T>
	struct Array
	{
		i64 size_ = 0;
		i64 max_size_ = 0;
		T* data_ = nullptr;

		Array()
		{
		}

		~Array()
		{
			Release();
		}

		Array(Array const& rhs)
		{
			ResizeUninitialised(rhs.Size());
			memcpy(data_, rhs.data_, Size());
		}

		Array& operator =(Array const& rhs)
		{
			Release();
			ResizeUninitialised(rhs.Size());
			memcpy(data_, rhs.data_, Size());
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

		Array& operator =(Array&& rhs)
		{
			data_ = rhs.data_;
			size_ = rhs.size_;
			max_size_ = rhs.max_size_;

			rhs.data_ = nullptr;
			rhs.max_size_ = rhs.size_ = 0;

			return *this;
		}

		void Reserve(i64 min_size)
		{
			i64 max_size = max_size_;

			if (max_size == 0)
			{
				max_size = min_size;
			}

			while (max_size < min_size)
			{
				if (max_size < 1024)
				{
					max_size *= 2;
				}
				else
				{
					max_size = max_size * 3 / 2;
				}
			}

			if (max_size != max_size_)
			{
				max_size_ = max_size;
				data_ = static_cast<T*>(realloc(data_, sizeof(T) * max_size_));
			}
		}

		void _Resize(i64 size, bool initialise)
		{
			Reserve(size);

			if (initialise)
			{
				for (i64 i = size_; i < size; i++)
				{
					new (data_ + i) T();
				}
			}

			if constexpr (!std::is_trivial_v<T>)
			{
				for (i64 i = size_ - 1; i >= size; i--)
				{
					(data_ + i)->T::~T();
				}
			}

			size_ = size;
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

		void Append(T const* src, i64 num)
		{
			static_assert(std::is_trivially_copyable_v<T>);
			Reserve(size_ + num);
			memcpy(data_ + size_, src, num * sizeof(T));
			size_ += num;
		}

		void Shrink()
		{
			if (max_size_ == size_)
			{
				return;
			}

			if (size_ == 0)
			{
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

		const T* Data()  const
		{
			return data_;
		}

		T* Data()
		{
			return data_;
		}

		const T& At(i64 index) const
		{
			DEBUG_ASSERT(0 <= index && index < size_, containers_module{});
			return data_[index];
		}

		T& At(i64 index)
		{
			DEBUG_ASSERT(0 <= index && index < size_, containers_module{});
			return data_[index];
		}

		const T& operator [](i64 index) const
		{
			return At(index);
		}

		T& operator [](i64 index)
		{
			return At(index);
		}

		T PopBack()
		{
			DEBUG_ASSERT(size_ > 0, containers_module{});
			size_ -= 1;
			if constexpr (std::is_trivially_copyable_v<T>)
			{
				return data_[size_];
			}

			static_assert(std::is_move_constructible_v<T>);
			return std::move(data_[size_]);
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
	};

}