#pragma once

#include "types.h"

namespace Hash
{
	template<typename T>
	u64 HashValue(T x);

	template<> u64 HashValue(i32 x);
	template<> u64 HashValue(i64 x);
	template<> u64 HashValue(u32 x);
	template<> u64 HashValue(u64 x);
}