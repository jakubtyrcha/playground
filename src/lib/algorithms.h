#pragma once

#include "types.h"

namespace Algorithms
{
	template<typename T>
	i64 LowerBound(const T* array, i64 size, T value)
	{
		i64 l = 0;
		i64 r = size;
		i64 c = r - l;
		while (c > 0)
		{
			i64 step = c / 2;
			if (array[l + step] < value) {
				l = l + step + 1;
				c -= step + 1;
			}
			else {
				c = step;
			}
		}

		return l;
	}

	template<typename T, typename = std::enable_if<std::is_trivially_copyable_v<T>>::type>
	T min(T l, T r) {
		if (r < l) {
			return r;
		}
		return l;
	}

	template<typename T, typename = std::enable_if<!std::is_trivially_copyable_v<T>>::type>
	const T& min(T const& l, T const& r) {
		if (r < l) {
			return r;
		}
		return l;
	}
}