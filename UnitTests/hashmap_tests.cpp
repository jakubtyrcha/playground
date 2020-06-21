#include "catch/catch.hpp"

#include "hashmap.h"
#include "copy_move.h"

using namespace Containers;

TEST_CASE("hashtable can be inserted and accessed", "[hashtable]") {
	Hashmap<i32, i32> h;

	int N = 100;

	for (int i = 0; i < N; i++)
	{
		REQUIRE(h.Insert(i, (i + 50) % 100) == true);
		REQUIRE(h.Size() == i + 1);
	}

	REQUIRE(h.Size() == N);

	for (int i = 0; i < N; i++)
	{
		REQUIRE(h.At(i) == (i + 50) % 100);
	}
}

TEST_CASE("can remove from hashtable", "[hashtable]") {
	Hashmap<i32, i32> h;

	int N = 100; // has to be even

	for (int i = 0; i < N; i++)
	{
		REQUIRE(h.Insert(i, (i + 50) % 100) == true);
		REQUIRE(h.Size() == i + 1);
	}

	REQUIRE(h.Size() == N);

	for (int i = 0; i < N; i += 2)
	{
		h.Remove(i);
	}

	REQUIRE(h.Size() == N / 2);

	for (int i = 1; i < N; i += 2)
	{
		REQUIRE(h.Contains(i));
	}

	for (int i = 0; i < N; i += 2)
	{
		REQUIRE(h.Contains(i) == false);
	}
}

TEST_CASE("hashtable can handle pow2", "[hashtable]") {
	Hashmap<i32, i32> h;

	int N = 10000;

	for (int i = 0; i < N; i++)
	{
		REQUIRE(h.Insert(i * 256, i));
	}

	REQUIRE(h.Size() == N);

	for (int i = 0; i < N; i++)
	{
		REQUIRE(h.At(i * 256) == i);
	}
}

TEST_CASE("hashtable can store ptrs", "[hashtable]") {
	Hashmap<i32*, i32> h;

	h.Insert(nullptr, 0);
}
