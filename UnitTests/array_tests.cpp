#include "catch/catch.hpp"

#include "array.h"

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