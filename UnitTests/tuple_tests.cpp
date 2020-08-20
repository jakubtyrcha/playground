#include "tuple.h"
#include "catch/catch.hpp"

using namespace Playground;

TEST_CASE("can create tuple", "[tuple]")
{
    Tuple<i32, f32> pair;
    REQUIRE(std::is_same_v<TupleElement<1, decltype(pair)>::Type, f32>);

    pair.Get<0>() = 4;
    pair.Get<1>() = 7.f;
}