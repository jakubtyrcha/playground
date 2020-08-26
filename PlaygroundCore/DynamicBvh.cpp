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

	Handle DynamicBvh::Add(Aabb3D bounds) {
		if(root_ == NULL_NODE) {
            i32 node = nodes_freelist_.Allocate();
            nodes_.ExpandToIndex(node);

            nodes_[node] = {
                    .bounds = bounds,
                    .parent = root_,
                    .children = { NULL_NODE, NULL_NODE }
                };
            
            root_ = node;

            return { node };
		}

        i32 index = root_;
        i32 new_leaf = NULL_NODE;

        // finding the parent
        while(true) {
            i32 children_num = nodes_[index].ChildrenNum();
            if(children_num == 2) {
                f32 score_left = nodes_[nodes_[index].children[0]].bounds.Union(bounds).Area();
                f32 score_right = nodes_[nodes_[index].children[1]].bounds.Union(bounds).Area();

                if (score_left <= score_right) {
                    index = nodes_[index].children[0];
                } else {
                    index = nodes_[index].children[1];
                }
            } else if(children_num == 1) {
                new_leaf = nodes_freelist_.Allocate();
                nodes_.ExpandToIndex(new_leaf);

                nodes_[new_leaf] = {
                    .bounds = bounds,
                    .parent = NULL_NODE,
                    .children = { NULL_NODE, NULL_NODE }
                };

                nodes_[index].children[1] = new_leaf;
                nodes_[new_leaf].parent = index;
                break;
            } else {
                // we got to the leaf
                // change parent -- leaf (index) into parent -- node -- leaf (index), new leaf
                // can't move the leaf because of the handles

                i32 split_node = nodes_freelist_.Allocate();
                nodes_.ExpandToIndex(split_node);

                new_leaf = nodes_freelist_.Allocate();
                nodes_.ExpandToIndex(new_leaf);

                nodes_[new_leaf] = {
                    .bounds = bounds,
                    .parent = NULL_NODE,
                    .children = { NULL_NODE, NULL_NODE }
                };

                nodes_[split_node] = {
                    .bounds = nodes_[index].bounds,
                    .parent = nodes_[index].parent,
                    .children = { index, new_leaf }
                };

                if (nodes_[index].parent != NULL_NODE) {
                    if (nodes_[nodes_[index].parent].children[1] == index) {
                        nodes_[nodes_[index].parent].children[1] = split_node;
                    } else {
                        nodes_[nodes_[index].parent].children[0] = split_node;
                    }
                } else {
                    root_ = split_node;
                }

                nodes_[index].parent = split_node;
                nodes_[new_leaf].parent = split_node;

                break;
            }
        }

        plgr_assert(new_leaf != NULL_NODE);
        _Refit(nodes_[new_leaf].parent);

        return { new_leaf };
	}

    f32 DynamicBvh::_GetMergeCost(i32 l, i32 r) const {
        Aabb3D merged = nodes_[l].bounds;
        plgr_assert(l != NULL_NODE);
        if(r != NULL_NODE) {
            merged = merged.Union(nodes_[r].bounds);
        }
        return merged.Area();
    }

    f32 DynamicBvh::_GetMergeCost(i32 i, i32 j, i32 k) const {
        plgr_assert(i != NULL_NODE && j != NULL_NODE);
        Aabb3D merged = nodes_[i].bounds.Union(nodes_[j].bounds);
        if(k != NULL_NODE) {
            merged = merged.Union(nodes_[k].bounds);
        }
        return merged.Area();
    }

    void DynamicBvh::Rotate(i32 index) {
        if(nodes_[index].ChildrenNum() < 2) {
            return;
        }

        i32 l = nodes_[index].children[0];
        i32 r = nodes_[index].children[1];

        if(nodes_[l].ChildrenNum() < nodes_[r].ChildrenNum()) {
            // make sure l has 2 children, less fuss later
            Swap(l, r);
            Swap(nodes_[index].children[0], nodes_[index].children[1]);
        }

        i32 l0 = nodes_[l].children[0];
        i32 l1 = nodes_[l].children[1];

        i32 r0 = nodes_[r].children[0];
        i32 r1 = nodes_[r].children[1];

        f32 AB = _GetMergeCost(l0, l1);
        f32 CD = _GetMergeCost(r0, r1);
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
            Aabb3D bounds = Aabb3D::Empty();
            if (nodes_[index].children[1] != NULL_NODE) {
                bounds = bounds.Union(nodes_[nodes_[index].children[1]].bounds);
            }
            if (nodes_[index].children[0] != NULL_NODE) {
                bounds = bounds.Union(nodes_[nodes_[index].children[0]].bounds);
            }
            if (nodes_[index].bounds.Contains(bounds)) {
                break;
            }
            nodes_[index].bounds = bounds;

            index = nodes_[index].parent;
        }
    }

    void DynamicBvh::Remove(Handle h) {
        i32 remove_index = h.index;
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
        nodes_freelist_.Free(remove_index);
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
                if (nodes_[node].children[1] != NULL_NODE) {
                    stack.PushBack({ .node = nodes_[node].children[1], .depth = depth + 1 });
                }
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
                    if (distance < best_distance) {
                        best_distance = distance;
                        best_option = Handle { node };
                    }
                    if (distance == 0.f) {
                        return Handle { node };
                    }
                } else {
                    stack.PushBack(nodes_[node].children[0]);
                    if (nodes_[node].children[1] != NULL_NODE) {
                        stack.PushBack(nodes_[node].children[1]);
                    }
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
                    out.PushBack(Handle { node });
                } else {
                    stack.PushBack(nodes_[node].children[0]);
                    if (nodes_[node].children[1] != NULL_NODE) {
                        stack.PushBack(nodes_[node].children[1]);
                    }
                }
            }
        }

        return out.Size();
    }
}