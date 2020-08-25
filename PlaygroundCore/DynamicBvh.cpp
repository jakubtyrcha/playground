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

        if(children[1] == NULL_NODE) {
            return 1;
        }

        return 2;
    }

    i32 DynamicBvh::Node::FreeChildIndex() const {
        if(children[0] == NULL_NODE) {
            return 0;
        }
        plgr_assert(children[1] == NULL_NODE);
        return 1;
    }

	Handle DynamicBvh::Add(Aabb3D bounds) {
		if(root_ == NULL_NODE) {
            root_ = nodes_freelist_.Allocate();
            nodes_.ExpandToIndex(root_);
            nodes_[root_] = {
                .bounds = bounds,
                .parent = NULL_NODE,
                .children = { NULL_NODE, NULL_NODE }
            };

            i32 node = nodes_freelist_.Allocate();
            nodes_.ExpandToIndex(node);

            nodes_[node] = {
                    .bounds = bounds,
                    .parent = root_,
                    .children = { NULL_NODE, NULL_NODE }
                };
            nodes_[root_].children[0] = node;

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

                if(nodes_[nodes_[index].parent].children[1] == index) {
                    nodes_[nodes_[index].parent].children[1] = split_node;
                } else {
                    nodes_[nodes_[index].parent].children[0] = split_node;
                }

                nodes_[index].parent = split_node;
                nodes_[new_leaf].parent = split_node;

                break;
            }
        }

        plgr_assert(new_leaf != NULL_NODE);

        i32 current = new_leaf;
        i32 parent = nodes_[current].parent;
        while(parent != NULL_NODE) {
            Aabb3D parent_extended_aabb = nodes_[parent].bounds.Union(nodes_[current].bounds);
            if(nodes_[parent].bounds == parent_extended_aabb) {
                break;
            }
            nodes_[parent].bounds = parent_extended_aabb;
            current = parent;
            parent = nodes_[parent].parent;
        }

        return { new_leaf };
	}

    void DynamicBvh::_Detach(i32 index) {
        plgr_assert(nodes_[index].ChildrenNum() == 0);
        i32 parent = nodes_[index].parent;
        if(parent == NULL_NODE) {
            root_ = NULL_NODE;
            return;
        }
        if (nodes_[parent].children[0] == index) {
            nodes_[parent].children[0] = nodes_[parent].children[1];
            nodes_[parent].children[1] = NULL_NODE;
        } else {
            nodes_[parent].children[1] = NULL_NODE;
        }
    }

    void DynamicBvh::_TrimNode(i32 index) {
        if(index == NULL_NODE) {
            return;
        }
        i32 children_num = nodes_[index].ChildrenNum();
        if(children_num == 0) {
            _Detach(index);
            _TrimNode(nodes_[index].parent);
            nodes_freelist_.Free(index);
        } else {
            // children node == 1
            i32 parent = nodes_[index].parent;
            if(parent == NULL_NODE) {
                // keep, root node
            } else {
                // this is a transient node, we can remove it
                if(nodes_[parent].children[0] == index) {
                    nodes_[parent].children[0] = nodes_[index].children[0];
                } else {
                    nodes_[parent].children[1] = nodes_[index].children[0];
                }
                nodes_[nodes_[index].children[0]].parent = parent;
                nodes_[parent].bounds = nodes_[nodes_[parent].children[0]].bounds;
                nodes_freelist_.Free(index);

                parent = nodes_[parent].parent;

                while(parent != NULL_NODE) {
                    Aabb3D bounds = nodes_[nodes_[parent].children[0]].bounds;
                    if(nodes_[parent].children[1]) {
                        bounds = bounds.Union(nodes_[nodes_[parent].children[1]].bounds);
                    }
                    if(nodes_[parent].bounds.Contains(bounds)) {
                        break;
                    }
                    nodes_[parent].bounds = bounds;

                    parent = nodes_[parent].parent;
                }
            }
        }
    }

    void DynamicBvh::Remove(Handle h) {
        i32 remove_index = h.index;
        i32 parent = nodes_[remove_index].parent;
        _Detach(remove_index);
        _TrimNode(parent);
        nodes_freelist_.Free(remove_index);
    }

    void DynamicBvh::Modify(Handle h, Aabb3D aabb) {
        
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