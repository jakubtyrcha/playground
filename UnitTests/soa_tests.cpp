#include "soa.h"
#include "catch/catch.hpp"

using namespace Playground;

TEST_CASE("can create Soa", "[soa]")
{
    struct Data {
        i32 x;
        i32 y;
        i32 z;
    };

    struct XYZ : public Soa<f32, f32, f32, Data> {
        enum Columns {
            X,
            Y,
            Z,
            Data
        };
    };
	XYZ xyz;

	xyz.PushBack();
	REQUIRE(xyz._ArrayFromIndex<XYZ::X>().Size() == 1);
	REQUIRE(xyz._ArrayFromIndex<XYZ::Y>().Size() == 1);
	REQUIRE(xyz._ArrayFromIndex<XYZ::Z>().Size() == 1);
    REQUIRE(xyz._ArrayFromIndex<XYZ::Data>().Size() == 1);

    xyz.AtMut<XYZ::X>(0) = 0.f;
    xyz.AtMut<XYZ::Y>(0) = -1.f;
    xyz.AtMut<XYZ::Z>(0) = 1.f;
    xyz.AtMut<XYZ::Data>(0) = { .x = 1, .y = 2, .z = 3 };

    REQUIRE(xyz.Size() == 1);

    xyz.PushBack();
    REQUIRE(xyz.Size() == 2);

    REQUIRE(xyz.DataSlice<XYZ::X>()[0] == 0.f);
    REQUIRE(xyz.DataSlice<XYZ::Y>()[0] == -1.f);
    REQUIRE(xyz.DataSlice<XYZ::Z>()[0] == 1.f);
    REQUIRE(xyz.DataSlice<XYZ::Data>()[0].x == 1);
    REQUIRE(xyz.DataSlice<XYZ::Data>()[0].y == 2);
    REQUIRE(xyz.DataSlice<XYZ::Data>()[0].z == 3);

    xyz.RemoveAt(1);
}
