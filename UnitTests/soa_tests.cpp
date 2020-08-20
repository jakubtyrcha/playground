#include "soa.h"
#include "catch/catch.hpp"

using namespace Playground;

TEST_CASE("can create Soa", "[soa]")
{
    struct XYZ : public Soa<f32, f32, f32> {
        enum Columns {
            X,
            Y,
            Z
        };
    };
	XYZ xyz;

	xyz.PushBack();
	REQUIRE(xyz._ArrayFromIndex<XYZ::X>().Size() == 1);
	REQUIRE(xyz._ArrayFromIndex<XYZ::Y>().Size() == 1);
	REQUIRE(xyz._ArrayFromIndex<XYZ::Z>().Size() == 1);

    xyz.AtMut<XYZ::X>(0) = 0.f;
    xyz.AtMut<XYZ::Y>(0) = -1.f;
    xyz.AtMut<XYZ::Z>(0) = 1.f;

    REQUIRE(xyz.Size() == 1);

    xyz.PushBack();
    REQUIRE(xyz.Size() == 2);

    REQUIRE(xyz.DataSlice<XYZ::X>()[0] == 0.f);
    REQUIRE(xyz.DataSlice<XYZ::Y>()[0] == -1.f);
    REQUIRE(xyz.DataSlice<XYZ::Z>()[0] == 1.f);

    xyz.RemoveAt(1);
}
