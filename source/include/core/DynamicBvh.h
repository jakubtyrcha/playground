#pragma once
#include "Geometry.h"
#include "FreeList.h"

namespace Playground {

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
        i32 FreeChildIndex() const;
    };

    static constexpr i32 NULL_NODE = -1;

    FreeList nodes_freelist_;
    Array<Node> nodes_;
    i32 root_ = NULL_NODE;

    Handle Add(Aabb3D bounds);
    void Modify(Handle, Aabb3D);
    void Remove(Handle);

    void _Detach(i32);
    void _TrimNode(i32);
    void _Refit(i32);

    i32 GetDepth() const;

    Optional<Handle> FindClosest(Vector3 point, f32 max_distance) const;
    bool FindAllIntersecting(Vector3 point, Array<Handle> & out) const;
};

}