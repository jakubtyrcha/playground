#include "hash.h"

namespace Hash
{
	template<> u64 HashValue(i32 x) { return x; }
	template<> u64 HashValue(i64 x) { return x; }
	template<> u64 HashValue(u32 x) { return x; }
	template<> u64 HashValue(u64 x) { return x; }
}