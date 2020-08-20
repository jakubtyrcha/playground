#pragma once
#include "Types.h"

namespace Playground {

template <i32 Index, typename Tuple>
struct TupleElement;

// https://medium.com/@mortificador/implementing-std-tuple-in-c-17-3cc5c6da7277

template <i32 _Index, typename _Type>
struct _TupleItem {
    using Type = _Type;
    constexpr static i32 Index = _Index;
    Type item_;

    _TupleItem() = default;

    _TupleItem(Type const & v) : item_(v) {
    }

    _TupleItem(Type && v) {
        item_ = std::move(v);
    }

    Type& _Get() {
        return item_;
    }
};

template< i32 Index, typename... Types >
struct TupleRecursiveImpl {
};

template<i32 Index, typename Head, typename... Tail>
struct TupleRecursiveImpl<Index, Head, Tail...> : 
	public _TupleItem<Index, Head>,
	public TupleRecursiveImpl<Index+1, Tail...>
{
    TupleRecursiveImpl() = default;
};

template <typename... Types>
struct Tuple : public TupleRecursiveImpl<0, Types...> {
    template<i32 Index> 
	using Type = typename TupleElement<Index, Tuple<Types...>>::Type;

    Tuple() = default;

    template <i32 Index>
    auto& Get()
    {
        return static_cast<_TupleItem<Index, Type<Index>>&>(*this)._Get();
    }
};

template <typename... Types>
Tuple<Types...> MakeTuple(Types&&... args) {

}

//

template <i32 Index, typename Head, typename... Tail>
struct TupleElement<Index, Tuple<Head, Tail...>>
    : TupleElement<Index - 1, Tuple<Tail...>> {
};

template <class Head, class... Tail>
struct TupleElement<0, Tuple<Head, Tail...>> {
    using Type = Head;
};

}