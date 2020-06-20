#include "hashmap.h"
#include "algorithms.h"

namespace Containers
{
	i64 GetHashmapSize(i64 slots)
	{
		constexpr i64 sizes[] = {
			5, 11, 17, 37, 67, 131, 257, 521, 1031, 1543, 2309, 3457, 5189, 7789, 11677,
			17497, 26249, 39367, 59051, 88589, 132859, 199289, 298937, 448421, 672629, 1008901,
			1513361, 2270071, 3405019, 5107549, 7661293, 11491957, 17237933, 25856869, 38785297,
			58177939, 87266917, 130900361, 196350533, 294525811, 441788689, 662683051, 994024553
		};

		DEBUG_ASSERT(slots <= sizes[_countof(sizes) - 1], containers_module{});

		return sizes[Algorithms::LowerBound(sizes, _countof(sizes), slots)];
	}

	i64 HashmapSlot(u64 hash, i64 mod)
	{
		switch (mod)
		{
		case 5: return hash % 5;
		case 11: return hash % 11;
		case 17: return hash % 17;
		case 37: return hash % 37;
		case 67: return hash % 67;
		case 131: return hash % 131;
		case 257: return hash % 257;
		case 521: return hash % 521;
		case 1031: return hash % 1031;
		case 1543: return hash % 1543;
		case 2309: return hash % 2309;
		case 3457: return hash % 3457;
		case 5189: return hash % 5189;
		case 7789: return hash % 7789;
		case 11677: return hash % 11677;
		case 17497: return hash % 17497;
		case 26249: return hash % 26249;
		case 39367: return hash % 39367;
		case 59051: return hash % 59051;
		case 88589: return hash % 88589;
		case 132859: return hash % 132859;
		case 199289: return hash % 199289;
		case 298937: return hash % 298937;
		case 448421: return hash % 448421;
		case 672629: return hash % 672629;
		case 1008901: return hash % 1008901;
		case 1513361: return hash % 1513361;
		case 2270071: return hash % 2270071;
		case 3405019: return hash % 3405019;
		case 5107549: return hash % 5107549;
		case 7661293: return hash % 7661293;
		case 11491957: return hash % 11491957;
		case 17237933: return hash % 17237933;
		case 25856869: return hash % 25856869;
		case 38785297: return hash % 38785297;
		case 58177939: return hash % 58177939;
		case 87266917: return hash % 87266917;
		case 130900361: return hash % 130900361;
		case 196350533: return hash % 196350533;
		case 294525811: return hash % 294525811;
		case 441788689: return hash % 441788689;
		case 662683051: return hash % 662683051;
		case 994024553: return hash % 994024553;
		}
		DEBUG_UNREACHABLE(containers_module{});
		return -1;
	}

	u32 FastLog2(u32 v)
	{
		static const int MultiplyDeBruijnBitPosition[32] =
		{
		  0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
		  8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
		};

		v |= v >> 1; // first round down to one less than a power of 2 
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;

		return MultiplyDeBruijnBitPosition[(u32)(v * 0x07C4ACDDU) >> 27];
	}
}