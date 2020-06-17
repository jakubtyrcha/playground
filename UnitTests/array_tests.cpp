#include "catch/catch.hpp"

#include "array.h"

using namespace Container;

TEST_CASE("arrays can be resized and accessed", "[array]") {
	Array<int> array;
	array.Resize(16);
	for (int i = 0; i < 16; i++)
	{
		array.At(i) = i;
	}
	array.Reserve(64);
	array.Resize(50);
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
	array.Resize(1024);
	array.At(511) = 1;
	array.Resize(512);
	array.Shrink();
	REQUIRE(array.At(511) == 1);
	REQUIRE(array.Size() == 512);
	array.Resize(0);
	array.Shrink();
}