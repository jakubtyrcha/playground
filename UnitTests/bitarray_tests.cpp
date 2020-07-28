#include "bitarray.h"

#include "catch/catch.hpp"

using namespace Playground;

TEST_CASE("bitarray can store bits", "[bitarray]")
{

    Bitarray b;
    b.Resize(1000);

    for (i32 i = 0; i < 1000; i++) {
        b.SetBit(i, (i % 3) == 0);
    }

    for (i32 i = 0; i < 1000; i++) {
        REQUIRE(b.GetBit(i) == ((i % 3) == 0));
    }
}

TEST_CASE("bitarray can give next set bit", "[bitarray]")
{

    Bitarray b;
    b.Resize(1000);

    REQUIRE(b.GetNextBitSet(0) == 1000);

    b.SetBit(0, true);

    b.SetBit(2, true);
    b.SetBit(3, true);
    b.SetBit(4, true);

    b.SetBit(63, true);
    b.SetBit(64, true);
    b.SetBit(65, true);

    b.SetBit(999, true);

    for (i32 i = 5; i < 63; i++) {
        REQUIRE(b.GetBit(i) == false);
    }

    REQUIRE(b.GetNextBitSet(0) == 0);
    REQUIRE(b.GetNextBitSet(1) == 2);
    REQUIRE(b.GetNextBitSet(2) == 2);
    REQUIRE(b.GetNextBitSet(3) == 3);
    REQUIRE(b.GetNextBitSet(4) == 4);
    REQUIRE(b.GetNextBitSet(5) == 63);
    REQUIRE(b.GetNextBitSet(63) == 63);
    REQUIRE(b.GetNextBitSet(64) == 64);
    REQUIRE(b.GetNextBitSet(65) == 65);
    REQUIRE(b.GetNextBitSet(66) == 999);
    REQUIRE(b.GetNextBitSet(999) == 999);
}

TEST_CASE("bitarray can test if any bit is set", "[bitarray]")
{
    Bitarray b;
    b.Resize(63);
    REQUIRE(b.AnyBitSet() == false);
    b.SetBit(0, true);
    REQUIRE(b.AnyBitSet() == true);
    b.SetBit(0, false);
    REQUIRE(b.AnyBitSet() == false);
    b.SetBit(62, true);
    REQUIRE(b.AnyBitSet() == true);
    b.Resize(62);
    REQUIRE(b.AnyBitSet() == false);
    b.Resize(1000);
    REQUIRE(b.AnyBitSet() == false);
    b.SetBit(999, true);
    REQUIRE(b.AnyBitSet() == true);
}