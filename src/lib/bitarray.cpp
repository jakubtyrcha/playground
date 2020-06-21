#include "pch.h"

#include "bitarray.h"

#include <intrin.h>
#include <string.h>

namespace Containers
{
	void Bitarray::Resize(i64 size)
	{
		i64 block_num = (size + 63) / 64;

		if (size_ < size) {
			i64 block_index = size_ / 64;
			i64 bit_index = size_ % 64;

			if (bit_index) {
				data_[block_index] &= ~((-1) << bit_index);
			}
		}

		data_.Resize(block_num);

		size_ = size;
	}

	void Bitarray::SetBit(i64 index, bool v)
	{
		DEBUG_ASSERT(index < size_, containers_module{});

		i64 block_index = index / 64;
		i64 bit_index = index % 64;

		if (v)
		{
			_bittestandset64(&data_[block_index], bit_index);
		}
		else
		{
			_bittestandreset64(&data_[block_index], bit_index);
		}
	}

	bool Bitarray::GetBit(i64 index) const
	{
		DEBUG_ASSERT(index < size_, containers_module{});

		i64 block_index = index / 64;
		i64 bit_index = index % 64;

		return _bittest64(&data_[block_index], bit_index);
	}

	bool Bitarray::AnyBitSet() const {
		for (i64 i = 0, N = data_.Size() - 1; i < N; i++) {
			if (__popcnt64(data_[i])) {
				return true;
			}
		}

		i64 value = data_[data_.Size() - 1] << (64 - (size_ % 64));
		return __popcnt64(value);
	}

	i64 Bitarray::GetNextBitSet(i64 start_index) const
	{
		DEBUG_ASSERT(start_index < size_, containers_module{});

		i64 block_index = start_index / 64;
		i64 bit_index = start_index % 64;

		unsigned long index;
		// check the first block
		if (_BitScanForward64(&index, data_[block_index] >> bit_index) && (start_index + index) < size_)
		{
			return start_index + index;
		}

		// check the middle blocks
		for (i64 b = block_index + 1, N = data_.size_ - 1; b < N; b++)
		{
			if (_BitScanForward64(&index, data_[b]))
			{
				return b * 64 + index;
			}
		}

		// check the last block
		if (block_index != (data_.Size() - 1) && _BitScanForward64(&index, data_[data_.Size() - 1]) && (data_.Size() - 1) * 64 + index < size_)
		{
			return (data_.Size() - 1) * 64 + index;
		}

		return size_;
	}

	void Bitarray::ClearAll() {
		memset(data_.Data(), 0, data_.Size() * sizeof(i64));
	}

	void Bitarray::Shrink() {
		data_.Shrink();
	}
}
