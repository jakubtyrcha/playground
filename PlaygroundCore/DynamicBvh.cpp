#include "Pch.h"
#include "DynamicBvh.h"

namespace Playground {
	using Handle = DynamicBvh::Handle;

    bool DynamicBvh::Node::IsLeaf() const {
        return ChildrenNum() == 0;
    }

    i32 DynamicBvh::Node::ChildrenNum() const {
        if(children[0] == -1) {
            plgr_assert(children[1] == NULL_NODE);
            return 0;
        }

        plgr_assert(children[1] != NULL_NODE);

        return 2;
    }

    i32 DynamicBvh::Node::GetChildIndex(i32 i) const {
        if (children[0] == i) {
            return 0;
        }
        if (children[1] == i) {
            return 1;
        }
        plgr_assert(false);
        return -1;
    }

    i32 DynamicBvh::Node::GetSibling(i32 i) const {
        if(children[0] == i) {
            return children[1];
        }
        return children[0];
    }

    i32 DynamicBvh::_Split(i32 index) {
        i32 split = nodes_freelist_.Allocate();
        nodes_.ExpandToIndex(split);

        nodes_[split] = {
                .bounds = Aabb3D::Empty(),
                .parent = nodes_[index].parent,
                .children = { index, NULL_NODE }
            };

        nodes_[index].parent = split;

        if(root_ == index) {
            root_ = split;
        }

        return split;
    }

    // branch & bound
    // https://box2d.org/files/ErinCatto_DynamicBVH_GDC2019.pdf

    i32 DynamicBvh::_Add(Aabb3D inflated_bounds, Optional<i32> index) {
        if(root_ == NULL_NODE) {
            i32 node;
            if(index) {
                node = *index;
            } else {
                node = nodes_freelist_.Allocate();
                nodes_.ExpandToIndex(node);
            }

            nodes_[node] = {
                    .bounds = inflated_bounds,
                    .parent = root_,
                    .children = { NULL_NODE, NULL_NODE }
                };
            
            root_ = node;

            return node;
		}

        i32 best_sibling = NULL_NODE;
        f32 best_cost = Math::Constants<f32>::inf();
        struct Frame {
            i32 index;
            f32 inherited_cost;
        };
        // TODO: prio queue
        Array<Frame> stack;
        stack.PushBack({ .index = root_, .inherited_cost = 0.f });

        f32 leaf_cost = inflated_bounds.Area();

        while(stack.Size()) {
            auto [index, inherited_cost] = stack.PopBack();

            f32 direct_cost = nodes_[index].bounds.Union(inflated_bounds).Area();
            f32 cost = direct_cost + inherited_cost;

            if(cost < best_cost) {
                best_cost = cost;
                best_sibling = index;
            }

            if(nodes_[index].IsLeaf()) {
                continue;
            }

            f32 subrees_inherited_cost = inherited_cost + nodes_[index].bounds.Area();
            f32 lower_bound = subrees_inherited_cost + leaf_cost;

            if(lower_bound < best_cost) {
                stack.PushBack({ .index = nodes_[index].children[0], .inherited_cost = subrees_inherited_cost });
                stack.PushBack({ .index = nodes_[index].children[1], .inherited_cost = subrees_inherited_cost });
            }
        }

        plgr_assert(best_sibling != NULL_NODE);

        i32 parent = _Split(best_sibling);

        i32 new_leaf;
        if (index) {
            new_leaf = *index;
        } else {
            new_leaf = nodes_freelist_.Allocate();
            nodes_.ExpandToIndex(new_leaf);
        }

        nodes_[new_leaf] = {
            .bounds = inflated_bounds,
            .parent = parent,
            .children = { NULL_NODE, NULL_NODE }
        };

        nodes_[parent].children[1] = new_leaf;

        plgr_assert(new_leaf != NULL_NODE);
        _Refit(nodes_[new_leaf].parent);

        return new_leaf;
    }

	Handle DynamicBvh::Add(Aabb3D bounds, InflationPolicy inflation_policy) {
		i32 new_leaf = _Add(_Inflate(bounds, inflation_policy), NullOpt);

        leaves_.ExpandToIndex(new_leaf);
        leaves_[new_leaf] = { .tight_bounds = bounds, .inflation_policy = inflation_policy };

        return { new_leaf };
	}

    f32 DynamicBvh::_GetMergeCost(i32 l, i32 r) const {
        plgr_assert(l != NULL_NODE && r != NULL_NODE);
        return nodes_[l].bounds.Union(nodes_[r].bounds).Area();
    }

    f32 DynamicBvh::_GetMergeCost(i32 i, i32 j, i32 k) const {
        plgr_assert(i != NULL_NODE && j != NULL_NODE && k != NULL_NODE);
        return nodes_[i].bounds.Union(nodes_[j].bounds).Union(nodes_[k].bounds).Area();
    }

    // https://hwrt.cs.utah.edu/papers/hwrt_rotations.pdf

    void DynamicBvh::_Rotate21(i32 n, i32 l) {
        plgr_assert(nodes_[n].ChildrenNum() == 2);
        plgr_assert(nodes_[l].ChildrenNum() == 0);

        i32 c0 = nodes_[n].children[0];
        i32 c1 = nodes_[n].children[1];

        f32 AB = nodes_[n].bounds.Area();
        f32 AC = _GetMergeCost(c0, l);
        f32 BC = _GetMergeCost(c1, l);

        if(AC < AB) {
            _Rotate(c1, l);
        } else if(BC < AB) {
            _Rotate(c0, l);
        }
    }

    void DynamicBvh::_Rotate22(i32 l, i32 r) {
        i32 l0 = nodes_[l].children[0];
        i32 l1 = nodes_[l].children[1];

        i32 r0 = nodes_[r].children[0];
        i32 r1 = nodes_[r].children[1];

        f32 AB = nodes_[l].bounds.Area();
        f32 CD = nodes_[r].bounds.Area();
        f32 AC = _GetMergeCost(l0, r0);
        f32 AD = _GetMergeCost(l0, r1);
        f32 BC = _GetMergeCost(l1, r0);
        f32 BD = _GetMergeCost(l1, r1);

        f32 ABC = _GetMergeCost(l0, l1, r0);
        f32 ABD = _GetMergeCost(l0, l1, r1);
        f32 ACD = _GetMergeCost(l0, r0, r1);
        f32 BCD = _GetMergeCost(l1, r0, r1);

        f32 cost_noop = AB + CD;
        f32 cost_swap_l0r0 = BC + AD;
        f32 cost_swap_l1r0 = AC + BD;
        f32 cost_swap_lr0 = AB + ABD;
        f32 cost_swap_lr1 = AB + ABC;
        f32 cost_swap_l0r = CD + BCD;
        f32 cost_swap_l1r = CD + ACD;

        f32 min_cost = Min(Min(cost_noop, Min(cost_swap_l0r0, cost_swap_l1r0)), Min(Min(cost_swap_lr0, cost_swap_lr1), Min(cost_swap_l0r, cost_swap_l1r)));

        if(cost_noop == min_cost) {
        }
        else if(cost_swap_l0r0 == min_cost) {
            _Rotate(l0, r0);
        }
        else if(cost_swap_l1r0 == min_cost) {
            _Rotate(l1, r0);
        }
        else if(cost_swap_lr0 == min_cost) {
            _Rotate(l, r0);
        }
        else if(cost_swap_lr1 == min_cost) {
            _Rotate(l, r1);
        }
        else if(cost_swap_l0r == min_cost) {
            _Rotate(l0, r);
        }
        else if(cost_swap_l1r == min_cost) {
            _Rotate(l1, r);
        }
    }

    void DynamicBvh::Rotate(i32 index) {
        if(nodes_[index].ChildrenNum() == 0) {
            return;
        }

        i32 l = nodes_[index].children[0];
        i32 r = nodes_[index].children[1];

        i32 children_l = nodes_[l].ChildrenNum();
        i32 children_r = nodes_[r].ChildrenNum();

        if(children_l == 0 && children_r == 2) {
            _Rotate21(r, l);
        }
        else if(children_l == 2 && children_r == 0) {
            _Rotate21(l, r);
        }
        else if(children_l == 2 && children_r == 2) {
            _Rotate22(l, r);
        }

        if(nodes_[index].parent != NULL_NODE) {
            Rotate(nodes_[index].parent);
        }
    }

    void DynamicBvh::_Rotate(i32 index0, i32 index1) {
        i32 parent0 = nodes_[index0].parent;
        i32 parent1 = nodes_[index1].parent;
        plgr_assert(parent0 != NULL_NODE && parent1 != NULL_NODE);
        plgr_assert(parent0 != parent1);

        i32 child0 = nodes_[parent0].GetChildIndex(index0);
        i32 child1 = nodes_[parent1].GetChildIndex(index1);

        // swap
        nodes_[parent0].children[child0] = index1;
        nodes_[index1].parent = parent0;
        nodes_[parent1].children[child1] = index0;
        nodes_[index0].parent = parent1;

        _Refit(parent0);
        _Refit(parent1);
    }

    void DynamicBvh::_Refit(i32 index) {

        while (index != NULL_NODE) {
            Aabb3D bounds = nodes_[nodes_[index].children[0]].bounds.Union(nodes_[nodes_[index].children[1]].bounds);
            if (nodes_[index].bounds.Contains(bounds)) {
                break;
            }
            nodes_[index].bounds = bounds;

            Rotate(index);
            index = nodes_[index].parent;
        }
    }

    void DynamicBvh::_Remove(i32 remove_index) {
        i32 parent = nodes_[remove_index].parent;
        if(parent == NULL_NODE) {
            root_ = NULL_NODE;
        } else {
            i32 parent_2 = nodes_[parent].parent;
            i32 sibling = nodes_[parent].GetSibling(remove_index);
            if(parent_2 == NULL_NODE) {
                root_ = sibling;
                nodes_[sibling].parent = NULL_NODE;
            } else {
                nodes_[sibling].parent = parent_2;
                nodes_[parent_2].children[nodes_[parent_2].GetChildIndex(parent)] = sibling;
                _Refit(parent_2);
            }
            nodes_freelist_.Free(parent);
        }
    }

    void DynamicBvh::Remove(Handle h) {
        i32 remove_index = h.index;
        _Remove(remove_index);
        nodes_freelist_.Free(remove_index);
    }

    Aabb3D DynamicBvh::_Inflate(Aabb3D bounds, InflationPolicy) {
        return bounds.Scaled(2.f);
    }

    void DynamicBvh::Modify(Handle current, Aabb3D bounds) {
        // remove & add while maintaining the handle alive
        i32 index = current.index;
        if(!nodes_[index].bounds.Contains(bounds)) {
            //
            _Remove(index);
            _Add(_Inflate(bounds, leaves_[index].inflation_policy), index);
        }
        leaves_[index].tight_bounds = bounds;
    }

    i32 DynamicBvh::GetDepth() const {
        if(root_ == NULL_NODE) {
            return 0;
        }
        struct Frame {
            i32 node;
            i32 depth;
        };
        Array<Frame> stack;
        stack.PushBack({.node = root_, .depth = 1});
        i32 max_depth = 1;

        while(stack.Size()) {
            auto [node, depth] = stack.PopBack();
            if (nodes_[node].IsLeaf()) {
                max_depth = Max(max_depth, depth);
            } else {
                stack.PushBack({ .node = nodes_[node].children[0], .depth = depth + 1 });
                stack.PushBack({ .node = nodes_[node].children[1], .depth = depth + 1 });
            }
        }

        return max_depth;
    }

    Optional<Handle> DynamicBvh::FindClosest(Vector3 point, f32 max_distance) const {
        if(root_ == NULL_NODE) {
            return NullOpt;
        }

        i32 node = root_;

        Array<i32> stack;
        stack.PushBack(node);

        f32 best_distance = Math::Constants<f32>::inf();
        Optional<Handle> best_option;

        while(stack.Size()) {
            i32 node = stack.PopBack();

            f32 distance = nodes_[node].bounds.Distance(point);

            if(distance <= max_distance) {
                if (nodes_[node].IsLeaf()) {
                    distance = leaves_[node].tight_bounds.Distance(point);

                    if (distance < best_distance && distance <= max_distance) {
                        best_distance = distance;
                        best_option = Handle { node };
                    }
                    if (distance == 0.f) {
                        return Handle { node };
                    }
                } else {
                    stack.PushBack(nodes_[node].children[0]);
                    stack.PushBack(nodes_[node].children[1]);
                }
            }
        }

        return best_option;
    }

    bool DynamicBvh::FindAllIntersecting(Vector3 point, Array<Handle> & out) const {
        plgr_assert(out.Size() == 0);

        if(root_ == NULL_NODE) {
            return false;
        }

        i32 node = root_;

        Array<i32> stack;
        stack.PushBack(node);

        while(stack.Size()) {
            i32 node = stack.PopBack();

            if (nodes_[node].bounds.Contains(point)) {
                if (nodes_[node].IsLeaf()) {
                    if (leaves_[node].tight_bounds.Contains(point)) {
                        out.PushBack(Handle { node });
                    }
                } else {
                    stack.PushBack(nodes_[node].children[0]);
                    stack.PushBack(nodes_[node].children[1]);
                }
            }
        }

        return out.Size();
    }
}