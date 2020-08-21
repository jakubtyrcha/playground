#pragma once
#include "DenseArray.h"
#include "SparseArray.h"
#include "Hashmap.h"

namespace Playground {

using ComponentTypeId = i32;

namespace CoreComponentTypes {
    constexpr ComponentTypeId Invalid = 0;
    constexpr ComponentTypeId Transform = 1; // pos, rotation, scale
    constexpr ComponentTypeId TransformPrev = 2;
    constexpr ComponentTypeId WorldAabb = 3; 
}

constexpr ComponentTypeId InvalidComponentT = CoreComponentTypes::Invalid;

struct TypelessComponentId {
    u32 id;

    bool operator == (TypelessComponentId other) const;
};

struct TypedComponentId {
    TypelessComponentId id = {};
    ComponentTypeId type = InvalidComponentT;

    bool operator == (TypedComponentId other) const {
        return id == other.id && type == other.type;
    }
};

template <ComponentTypeId _ComponentTypeId>
struct ComponentId : public Handle32 {
    using Type = ComponentId<_ComponentTypeId>; 
    using BaseType = Handle32;
    static constexpr ComponentTypeId ComponentTypeId = _ComponentTypeId;

    static ComponentId Make(i32 index, i32 generation)
    {
        return { BaseType::Make(index, generation) };
    }

    operator TypelessComponentId() const
    {
        return { .id = value_ };
    }

    operator TypedComponentId() const
    {
        return { .id = value_, .type = ComponentTypeId };
    }

    static Type From(TypelessComponentId typeless)
    {
        Type result {};
        result.value_ = typeless.id;
        return result;
    }

    bool operator ==(Type other) const {
        return As<BaseType>(*this) == other;
    }

    operator bool() const {
        return As<bool>(As<BaseType>(*this));
    }
};

using TransformComponentId = ComponentId<CoreComponentTypes::Transform>;

struct EntityId : public Handle32 {
    using BaseType = Handle32;
    static EntityId Make(i32 index, i32 generation);

    bool operator == (EntityId other) const {
        return As<BaseType>(*this) == As<BaseType>(other);
    }
};

struct Entity {
    TransformComponentId transform_ = {};

    i32 components_list_head_index = -1;
};

struct EntityComponentListNode {
    static constexpr i32 NODE_COMPONENTS_NUM = 8;
    static constexpr i32 EMPTY_NODE_INDEX = -1;

    TypedComponentId node_ids[NODE_COMPONENTS_NUM] = {};

    i32 next_node_index = EMPTY_NODE_INDEX;
    bool Empty() const;
};

struct Entities {
    // interface
    EntityId Spawn();
    void Destroy(EntityId);

    // can attach multiple components of the same type
    void AttachComponent(EntityId, TypedComponentId);
    //template<ComponentType ComponentT> void AttachComponent(EntityId, ComponentId<ComponentT>);

    void DetachComponent(EntityId, TypedComponentId);

    Optional<TypelessComponentId> GetAnyComponentOfType(EntityId, ComponentTypeId);

    // reverse lookup
    Optional<EntityId> GetOwner(TypedComponentId);

    DenseArray<Entity, EntityId> entities_;
    SparseArray<EntityComponentListNode> component_list_nodes_;

    Hashmap<TypedComponentId, EntityId> component_owner_; // TODO: split by type for faster lookups?
};

}