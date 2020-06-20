#include "bitarray.h"

#include <intrin.h>

namespace Containers
{
	void Bitarray::Resize(i64 size)
	{
		i64 block_num = (size + 63) / 64;
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

	void Bitarray::Shrink()
	{
		data_.Shrink();
	}
}
