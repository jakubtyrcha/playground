#pragma once

#include "Types.h"
#include "DenseArray.h"
#include "Soa.h"
#include "Entities.h"

namespace Playground {

template <typename _ComponentIdType, typename _Indexer>
struct ComponentContainer {
    using ComponentIdType = _ComponentIdType;

    _Indexer indexer_;

    ComponentIdType Add()
    {
        return indexer_.Add();
    }

    void Remove(ComponentIdType in)
    {
        indexer_.Remove(in);
    }

    template<i32 InnerIndex>
    auto& AtMut(ComponentIdType id)
    {
        return indexer_.AtMut<InnerIndex>(id);
    }

    template <i32 Index>
    auto DataSlice()
    {
        return indexer_.DataSlice<Index>();
    }

    i64 Size() const
    {
        return indexer_.Size();
    }

    ComponentIdType GetComponentFromFlatIndex(i32 index) {
        return indexer_.GetIdFromFlatIndex(index);
    }
};

template<typename ComponentId, typename ... Data>
using DenseComponentArray = ComponentContainer<ComponentId, DenseIndex<ComponentId, Soa<Data...>>>;

}