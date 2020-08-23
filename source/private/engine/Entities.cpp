#include "Pch.h"
#include "Entities.h"
#include "Core.h"

namespace Playground {

namespace Hash {
    template <>
    u64 HashValue(TypelessComponentId id)
    {
        return id.id;
    }
}

namespace Hash {
    template <>
    u64 HashValue(TypedComponentId id)
    {
        static_assert(sizeof(id.type) <= 4);
        static_assert(sizeof(id.id) <= 4);
        return (As<u64>(id.id.id) << 32) | As<u64>(id.type);
    }
}


bool TypelessComponentId::operator==(TypelessComponentId other) const
{
    return id == other.id;
}

EntityId EntityId::Make(i32 index, i32 generation)
{
    return EntityId { BaseType::Make(index, generation) };
}

EntityId Entities::Spawn()
{
    return entities_.Insert({});
}

void Entities::Destroy(EntityId id)
{
    entities_.Remove(id);
}

void Entities::AttachComponent(EntityId entity, TypedComponentId ctyped)
{
    plgr_assert(ctyped.type != InvalidComponentT);
    plgr_assert(!GetOwner(ctyped));
    if (ctyped.type == ComponentId<0>::ComponentTypeId) {
        plgr_assert(!entities_[entity].transform_);
        entities_[entity].transform_ = ComponentId<0>::From(ctyped.id);
    } else {
        i32 node_index = entities_[entity].components_list_head_index;
        if(node_index == EntityComponentListNode::EMPTY_NODE_INDEX) {
            node_index = component_list_nodes_.Allocate();
            entities_[entity].components_list_head_index = node_index;
        }

        bool attached = false;

        while (!attached) {
            for (i32 i = 0; i < EntityComponentListNode::NODE_COMPONENTS_NUM; i++) {
                if (component_list_nodes_[node_index].node_ids[i].type == InvalidComponentT) {
                    component_list_nodes_[node_index].node_ids[i] = ctyped;
                    attached = true;
                    break;
                }
            }

            if (!attached) {
                i32 next_node_index = component_list_nodes_[node_index].next_node_index;
                if(next_node_index == EntityComponentListNode::EMPTY_NODE_INDEX) {
                    next_node_index = component_list_nodes_.Allocate();
                    component_list_nodes_[node_index].next_node_index = next_node_index;
                }
                node_index = next_node_index;
            }
        }

        plgr_assert(attached);
    }

    component_owner_.Insert(ctyped, entity);
}

bool EntityComponentListNode::Empty() const
{
    for (i32 i = 0; i < EntityComponentListNode::NODE_COMPONENTS_NUM; i++) {
        if (node_ids[i].type != InvalidComponentT) {
            return false;
        }
    }

    return true;
}

void Entities::DetachComponent(EntityId entity, TypedComponentId ctyped)
{
    plgr_assert(ctyped.type != InvalidComponentT);
    plgr_assert(GetOwner(ctyped));
    if (ctyped.type == ComponentId<0>::ComponentTypeId) {
        plgr_assert(entities_[entity].transform_);
        entities_[entity].transform_ = {};
    } else {
        i32 node_index = entities_[entity].components_list_head_index;
        bool detached = false;

        {
            EntityComponentListNode& node = component_list_nodes_[node_index];
            for (i32 i = 0; i < EntityComponentListNode::NODE_COMPONENTS_NUM; i++) {
                if (node.node_ids[i] == ctyped) {
                    node.node_ids[i] = {};
                    detached = true;
                    break;
                }
            }
            if(detached && node.Empty()) {
                entities_[entity].components_list_head_index = node.next_node_index;
                component_list_nodes_.Free(node_index);
            }
        }

        i32 prev_node_index = node_index;
        node_index = component_list_nodes_[node_index].next_node_index;

        while (!detached && node_index != EntityComponentListNode::EMPTY_NODE_INDEX) {
            EntityComponentListNode& node = component_list_nodes_[node_index];
            for (i32 i = 0; i < EntityComponentListNode::NODE_COMPONENTS_NUM; i++) {
                if (node.node_ids[i] == ctyped) {
                    node.node_ids[i] = {};
                    detached = true;
                    break;
                }
            }

            i32 next_node_index = node.next_node_index;

            if (detached && node.Empty()) {
                component_list_nodes_[prev_node_index].next_node_index = next_node_index;
                component_list_nodes_.Free(node_index);
            }

            prev_node_index = node_index;
            node_index = next_node_index;
        }

        plgr_assert(detached);
    }

    component_owner_.Remove(ctyped);
}

Optional<TypelessComponentId> Entities::GetAnyComponentOfType(EntityId entity, ComponentTypeId ctype)
{
    plgr_assert(ctype != InvalidComponentT);
    if (ctype == ComponentId<0>::ComponentTypeId) {
        return As<TypelessComponentId>(entities_[entity].transform_);
    } else {
        i32 node_index = entities_[entity].components_list_head_index;

        while (node_index != EntityComponentListNode::EMPTY_NODE_INDEX) {
            EntityComponentListNode& node = component_list_nodes_[node_index];
            for (i32 i = 0; i < EntityComponentListNode::NODE_COMPONENTS_NUM; i++) {
                if (node.node_ids[i].type == ctype) {
                    return node.node_ids[i].id;
                    break;
                }
            }
            node_index = node.next_node_index;
        }
    }
    return NullOpt;
}

template<typename T>
Optional<T> UnpackPointer(Optional<T*> o) {
    if(!o) {
        return NullOpt;
    }
    return **o;
}

Optional<EntityId> Entities::GetOwner(TypedComponentId ctyped)
{
    return UnpackPointer(component_owner_.Find(ctyped));
}

}