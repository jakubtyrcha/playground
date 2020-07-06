#include "array.h"
#include "copy_move.h"

#include "catch/catch.hpp"

using namespace Containers;

TEST_CASE("arrays can be resized and accessed", "[array]") {
	Array<int> array;
	array.ResizeUninitialised(16);
	for (int i = 0; i < 16; i++)
	{
		array.At(i) = i;
	}
	array.Reserve(64);
	array.ResizeUninitialised(50);
	for (int i = 16; i < 50; i++)
	{
		array.At(i) = i;
	}

	for (int i = 0; i < 50; i++)
	{
		REQUIRE(array.At(i) == i);
	}
}

TEST_CASE("arrays can be shrinked", "[array]")
{
	Array<int> array;
	array.ResizeUninitialised(1024);
	array.At(511) = 1;
	array.ResizeUninitialised(512);
	array.Shrink();
	REQUIRE(array.At(511) == 1);
	REQUIRE(array.Size() == 512);
	array.ResizeUninitialised(0);
	array.Shrink();
}

TEST_CASE("arrays can be store moveable objects", "[array]")
{
	struct MoveMe : private MoveableNonCopyable<MoveMe>
	{
		int* ptr = nullptr;

		MoveMe() = default;
		MoveMe(int* x) : ptr(x) { if (ptr) { (*ptr)++; } }
		~MoveMe() { if (ptr) { (*ptr)--; } }

		MoveMe(MoveMe&& other) { ptr = other.ptr; other.ptr = nullptr; }
		MoveMe& operator=(MoveMe&& other) { ptr = other.ptr; other.ptr = nullptr; return *this; }
	};

	int x = 0;

	Array<MoveMe> array;
	for (int i = 0; i < 1000; i++)
	{
		array.PushBackRvalueRef(MoveMe(&x));
	}

	REQUIRE(x == 1000);

	while (array.Size())
	{
		array.PopBack();
	}

	REQUIRE(x == 0);
}
