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

    bvh.Add(Aabb3D::From(Vector3{ 0.f }, Vector3{ 1.f }));
    REQUIRE(bvh.FindClosest({}, 1.f));
}

TEST_CASE("test inclusive intervals in bvh", "[dynamic_bvh]")
{
    DynamicBvh bvh;
    
    Array<DynamicBvh::Handle> stack;

    // [0, 1], [0, 0.5], [0, 0.25], ...

    i32 subdivs = 15;
    stack.PushBack(bvh.Add(Aabb3D::From({}, Vector3{1.f})));
    f32 w = 0.5f;
    for(i32 i=0; i<subdivs; i++) {
        stack.PushBack(bvh.Add(Aabb3D::From({}, {w, 1.f, 1.f})));
        w *= 0.5f;
    }

    Array<DynamicBvh::Handle> collisions;
    bvh.FindAllIntersecting({}, collisions);
    REQUIRE(collisions.Size() == stack.Size());

    collisions.Clear();
    bvh.FindAllIntersecting(Vector3{1.f}, collisions);
    REQUIRE(collisions.Size() == 1);

    collisions.Clear();
    bvh.FindAllIntersecting(Vector3{0.25f, 0.f, 0.f}, collisions);
    REQUIRE(collisions.Size() == 3);

    i32 depth = bvh.GetDepth();

    REQUIRE(depth >= As<i32>(ceilf(log2f(As<f32>(subdivs + 1)))));
    REQUIRE(depth <= subdivs + 1);
}

TEST_CASE("test exclusive intervals in bvh", "[dynamic_bvh]")
{
    DynamicBvh bvh;
    
    Array<DynamicBvh::Handle> stack;

    // (almost exclusive) [0, 1] [1, 1.5] [1.5, 1.75] ...

    stack.PushBack(bvh.Add(Aabb3D::From({0.f, 0.f, 0.f}, Vector3{1.f, 1.f, 1.f})));
    f32 x = 1.f;
    f32 span = 0.5f;
    i32 subdivs = 15;
    for(i32 i=0; i<subdivs; i++) {
        stack.PushBack(bvh.Add(Aabb3D::From({x, 0.f, 0.f}, Vector3{x + span, 1.f, 1.f})));
        x += span;
        span *= 0.5f;
    }

    Array<DynamicBvh::Handle> collisions;

    bvh.FindAllIntersecting({}, collisions);
    REQUIRE(collisions.Size() == 1);
    collisions.Clear();

    bvh.FindAllIntersecting({1.f, 0.f, 0.f}, collisions);
    REQUIRE(collisions.Size() == 2);
    collisions.Clear();

    bvh.FindAllIntersecting({1.75f, 0.f, 0.f}, collisions);
    REQUIRE(collisions.Size() == 2);
    collisions.Clear();

    i32 depth = bvh.GetDepth();

    REQUIRE(depth >= As<i32>(ceilf(log2f(As<f32>(subdivs + 1)))));
    REQUIRE(depth <= subdivs + 1);
}

// TODO: move to benchmark
TEST_CASE("sorted insertion into dynamic bvh", "[dynamic_bvh]")
{
    DynamicBvh bvh;
    
    f32 x = 0.f;
    f32 span = 1.f;
    i32 boxes = 512;
    for(i32 i=0; i<boxes; i++) {
        bvh.Add(Aabb3D::From({x, 0.f, 0.f}, Vector3{x + span, 1.f, 1.f}));
        x += span;
    }

    i32 depth = bvh.GetDepth();

    REQUIRE(depth >= As<i32>(ceilf(log2f(As<f32>(boxes)))));
    REQUIRE(depth <= boxes);
}