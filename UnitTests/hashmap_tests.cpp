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

TEST_CASE("hashtable reinsert test for linear probing", "[hashtable]") {
	Hashmap<i32, i32> h;

	for (int i = 0; i < 10; i++) {
		h.Insert(i, i);
	}
	for (int i = 0; i < 10; i++) {
		h.Remove(i);
	}

	REQUIRE(h.Size() == 0);

	h.Insert(0, 0);
	int mod = static_cast<int>(h.Capacity());

	int collision2 = mod * 2;

	h.Insert(1, 1);
	h.Insert(collision2, 2);

	REQUIRE(h.Size() == 3);

	h.Remove(0);
	REQUIRE(h.Insert(collision2, 2) == false);
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
