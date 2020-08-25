#include "DynamicBvh.h"
#include "catch/catch.hpp"

using namespace Playground;

TEST_CASE("can create dynamic bvh and do point queries", "[dynamic_bvh]")
{
    DynamicBvh bvh;
    
    bvh.Add(Aabb3D::From(Vector3{ 0.f }, Vector3{ 1.f }));
    bvh.Add(Aabb3D::From(Vector3{ 0.f }, Vector3{ 2.f }));
    bvh.Add(Aabb3D::From(Vector3{ 1.f }, Vector3{ 2.f }));
    bvh.Add(Aabb3D::From(Vector3{ 0.f }, Vector3{ 4.f }));

    REQUIRE(bvh.FindClosest({}, 1.f));
    REQUIRE(bvh.FindClosest(Vector3{4.f}, 1.f));
    REQUIRE(bvh.FindClosest(Vector3{5.f, 4.f, 4.f}, 1.f));
    REQUIRE(!bvh.FindClosest(Vector3{5.f}, 0.5f));
    REQUIRE(!bvh.FindClosest(Vector3{10.f}, 1.f));
}

TEST_CASE("can remove points from dynamic bvh", "[dynamic_bvh]")
{
    DynamicBvh bvh;
    
    DynamicBvh::Handle h0 = bvh.Add(Aabb3D::From(Vector3{ 0.f }, Vector3{ 1.f }));
    DynamicBvh::Handle h1 = bvh.Add(Aabb3D::From(Vector3{ 0.f }, Vector3{ 2.f }));
    DynamicBvh::Handle h2 = bvh.Add(Aabb3D::From(Vector3{ 1.f }, Vector3{ 2.f }));
    DynamicBvh::Handle h3 = bvh.Add(Aabb3D::From(Vector3{ 0.f }, Vector3{ 4.f }));

    bvh.Remove(h0);
    REQUIRE(bvh.FindClosest({}, 1.f));
    bvh.Remove(h1);
    REQUIRE(bvh.FindClosest({}, 1.f));
    bvh.Remove(h3);
    REQUIRE(!bvh.FindClosest({}, 1.f));
    REQUIRE(!bvh.FindClosest(Vector3{3.f}, 1.f));
    bvh.Remove(h2);
    REQUIRE(!bvh.FindClosest(Vector3{2.f}, 1.f));


}