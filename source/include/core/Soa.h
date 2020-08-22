#pragma once
#include "Types.h"
#include "Array.h"
#include "Tuple.h"
#include "Slice.h"

namespace Playground {

// https://blog.tartanllama.xyz/exploding-tuples-fold-expressions/

template< i32 _Index, typename _Type>
struct SoaArray {
	using Type = _Type;
	constexpr static i32 Index = _Index;
	Array<Type> array_;

	Array<Type> & _Array()
    {
		return array_;
    }

	const Array<Type> & _Array() const
    {
		return array_;
    }
};

template< i32 Index, typename... Types >
struct SoaRecursiveImpl {
};

template<i32 Index, typename Head, typename... Tail>
struct SoaRecursiveImpl<Index, Head, Tail...> : 
	public SoaArray<Index, Head>,
	public SoaRecursiveImpl<Index+1, Tail...>
{
};

template<typename ... Types>
struct Soa : public SoaRecursiveImpl<0, Types...> {
	template<i32 Index> 
	using Type = typename TupleElement<Index, Tuple<Types...>>::Type;

	static constexpr i32 NumArrays = sizeof...(Types);

	template <i32 Index>
    auto& _ArrayFromIndex()
    {
        return static_cast<SoaArray<Index, Type<Index>>&>(*this)._Array();
    }

    template <i32 Index>
    const auto& _ArrayFromIndex() const
    {
        return static_cast<const SoaArray<Index, Type<Index>>&>(*this)._Array();
    }

	template <typename F, i32... Index>
    void _ForEachLoop(F && f, std::index_sequence<Index...>)
    {
        (f(_ArrayFromIndex<Index>(), std::integral_constant<i32, Index>()), ...);
    }

    template <typename F>
    void _ForEach(F && f)
    {
        _ForEachLoop(std::forward<F>(f), std::make_index_sequence<NumArrays>());
    }

    void ExpandToIndex(i32 index)
    {
        _ForEach([index](auto & array, auto vindex) { array.ExpandToIndex(index); });
    }

    void PushBackUninitialised()
    {
        _ForEach([](auto & array, auto vindex) { array.PushBackUninitialised(); });
    }

    void RemoveAt(i64 index)
    {
        _ForEach([index](auto & array, auto vindex) { array.RemoveAt(index); });
    }

    void RemoveAtAndSwapWithLast(i64 index)
    {
        _ForEach([index](auto & array, auto vindex) { array.RemoveAtAndSwapWithLast(index); });
    }

    template <i32 Index>
    Type<Index>& AtMut(i32 index)
    {
        return _ArrayFromIndex<Index>().At(index);
    }

    template <i32 Index>
    Slice<Type<Index>> DataSlice()
    {
        return {
            .data = _ArrayFromIndex<Index>().Data(), 
            .num = Size()
        };
    }

    i64 Size() const
    {
        // TODO: this is a tiny bit wasteful to keep all arrays separate, but I can live with it
        return _ArrayFromIndex<0>().Size();
    }
};

}