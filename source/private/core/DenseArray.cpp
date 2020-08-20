#include "Pch.h"
#include "DenseArray.h"

namespace Playground {

Handle32 Handle32::Make(i32 index, i32 generation)
{
	plgr_assert(index <= MAX_INDEX);
	i32 stored_generation = generation & MAX_GENERATION;
	plgr_assert(stored_generation); // 0 is a null value

	return { .value_ = As<u32>(index | (stored_generation << INDEX_BITS)) };
}

bool Handle32::operator()() const
{
	return !IsNull();
}

bool Handle32::IsNull() const
{
	return value_ == 0;
}

i32 Handle32::GetIndex() const
{
	return value_ & MAX_INDEX;
}

i32 Handle32::GetGeneration() const
{
	return (value_ >> INDEX_BITS);
}

}