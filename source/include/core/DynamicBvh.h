#pragma once
#include "Geometry.h"
#include "FreeList.h"

namespace Playground {

// TODO:
// when the queries are mostly 2D, does including 3rd dimension in cost help? probably not...

struct DynamicBvh {
    struct Handle {
        i32 index;
    };

    struct Node {
        Aabb3D bounds;
        i32 parent;
        i32 children[2];

        bool IsLeaf() const;
        i32 ChildrenNum() const;
        i32 GetChildIndex(i32) const;
        i32 GetSibling(i32) const;
    };

    enum InflationPolicy : i32 {
        Default
    };

    struct Leaf {
        Aabb3D tight_bounds;
        InflationPolicy inflation_policy;
    };

    static constexpr i32 NULL_NODE = -1;

    FreeList nodes_freelist_;
    Array<Node> nodes_;
    Array<Leaf> leaves_;
    i32 root_ = NULL_NODE;

    Handle Add(Aabb3D bounds, InflationPolicy inflation_policy = InflationPolicy::Default);
    void Remove(Handle);

    i32 _Add(Aabb3D inflated_bounds, Optional<i32> index);
    void _Remove(i32);

    void Modify(Handle current, Aabb3D bounds);

    Aabb3D _Inflate(Aabb3D, InflationPolicy);
    
    // make a temporary transitional node that should be patched with the new sibling
    i32 _Split(i32);
    // recursively walks up and update bounding boxes
    void _Refit(i32);
    void _Rotate(i32, i32);
    //    o 
    //  o   o
    // o o o o
    void _Rotate22(i32, i32);
    //    o 
    //  o   o
    // o o o x
    void _Rotate21(i32, i32);
    // this finds the optimal rotation of children (degree 1st and 2nd)
    // checks all 6 rotations and applies the best if better than the current setup
    void Rotate(i32);

    f32 _GetMergeCost(i32 l, i32 r) const;
    f32 _GetMergeCost(i32 i, i32 j, i32 k) const;

    i32 GetDepth() const;

    Optional<Handle> FindClosest(Vector3 point, f32 max_distance) const;
    bool FindAllIntersecting(Vector3 point, Array<Handle> & out) const;
};

}