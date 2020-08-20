#include "DenseArray.h"
#include "catch/catch.hpp"

using namespace Playground;

TEST_CASE("densearray can be accessed with 32 bit handle", "[densearray]")
{
    DenseArray<f32, Handle32> d;

    Handle32 v0 = d.Insert(0.f);
    Handle32 v1 = d.Insert(1.f);
    Handle32 v2 = d.Insert(2.f);

    REQUIRE(d.Size() == 3);

    REQUIRE(d[v0] == 0.f);
    REQUIRE(d[v1] == 1.f);
    REQUIRE(d[v2] == 2.f);

    d.Remove(v1);

    REQUIRE(d[v0] == 0.f);
    REQUIRE(d[v2] == 2.f);
}

TEST_CASE("densearray medium random access test", "[densearray]")
{
    DenseArray<i32, Handle32> d;

    Array<Handle32> handles;
    Array<i32> values;

    for(i32 i=0; i<1000;i++) {
        handles.PushBack(d.Insert(i));
        values.PushBack(i);
    }

    for(i32 i= 3; i< handles.Size(); i += 4) {
        d.Remove(handles[i]);
        handles.RemoveAtAndSwapWithLast(i);
        values.RemoveAtAndSwapWithLast(i);
    }

    for(i32 i=0; i<handles.Size(); i++) {
        REQUIRE(d[handles[i]] == values[i]);
    }
}
