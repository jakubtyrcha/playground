#pragma once

#include "Types.h"
#include "DenseArray.h"
#include "Soa.h"
#include "Entities.h"

namespace Playground {

template<typename _IdType, typename _ContainerType>
struct DenseIndex
{
    using IdType = _IdType;
    using ContainerType = _ContainerType;

    IdType Insert()
    {
    }

    void Remove(IdType id)
    {
    }

    template<i32 InnerIndex>
    auto& AtMut(IdType);

    i64 Size() const;

    ContainerType container_;
};

template <typename _IdType, typename ... Data>
struct ComponentContainer {
    using IdType = _IdType;

    DenseIndex<IdType, Soa<Data...>> data_;

    // virtual interface?
    // global array id -> interface
};

}