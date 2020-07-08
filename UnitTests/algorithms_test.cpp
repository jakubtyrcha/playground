#include "algorithms.h"
#include "catch/catch.hpp"

using namespace Algorithms;

TEST_CASE("lowerbound can be found", "[lowerbound]")
{
    int test[] = { 1, 2, 3, 4, 5 };
    REQUIRE(LowerBound(test, _countof(test), 1) == 0);
    REQUIRE(LowerBound(test, _countof(test), 3) == 2);
    REQUIRE(LowerBound(test, _countof(test), 5) == 4);

    int test1[] = { 1, 1, 1, 2, 2, 2, 3, 3, 3 };
    REQUIRE(LowerBound(test1, _countof(test1), 1) == 0);
    REQUIRE(LowerBound(test1, _countof(test1), 2) == 3);
    REQUIRE(LowerBound(test1, _countof(test1), 3) == 6);

    int test2[] = { 1, 1, 1, 1, 1, 1, 1, 2 };
    REQUIRE(LowerBound(test2, _countof(test2), 1) == 0);
    REQUIRE(LowerBound(test2, _countof(test2), 2) == 7);
}
