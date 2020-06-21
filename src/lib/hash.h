#pragma once

#include "types.h"

namespace Hash
{
	template<typename T>
	u64 HashValue(T);

	template<> u64 HashValue(i32);
	template<> u64 HashValue(i64);
	template<> u64 HashValue(u32);
	template<> u64 HashValue(u64);

	template<typename T> u64 HashValue(T* x)
	{
		return HashValue(reinterpret_cast<u64>(x));
	}

	u64 HashMemory(const void*, i64);
	u64 HashMemoryWithSeed(const void*, i64, u64);
}