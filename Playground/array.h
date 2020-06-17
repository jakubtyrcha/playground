#pragma once

#include <debug_assert/debug_assert.hpp>
#include "types.h"
#include <type_traits>

namespace Container
{
	struct container_module
		: debug_assert::default_handler, // use the default handler
		debug_assert::set_level<-1> // level -1, i.e. all assertions, 0 would mean none, 1 would be level 1, 2 level 2 or lower,...
	{};


	template<typename T>
	struct Array
	{
		i64 size_ = 0;
		i64 max_size_ = 0;
		T* data_ = nullptr;

		Array()
		{
			static_assert(std::is_trivial_v<T> == true);
		}

		~Array()
		{
			Release();
		}

		Array(Array const& rhs)
		{
			Resize(rhs.Size());
			memcpy(data_, rhs.data_, Size());
		}

		Array& operator =(Array const& rhs)
		{
			Release();
			Resize(rhs.Size());
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

		void Resize(i64 size)
		{
			Reserve(size);

			size_ = size;
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
			free(data_);
			data_ = nullptr;
			max_size_ = 0;
		}

		T* Data()
		{
			return data_;
		}

		const T* Data()  const
		{
			return data_;
		}

		T& At(i64 index)
		{
			DEBUG_ASSERT(0 <= index && index < size_, container_module{});
			return data_[index];
		}

		const T& At(i64 index) const
		{
			DEBUG_ASSERT(0 <= index && index < size_, container_module{});
			return data_[index];
		}
	};

}