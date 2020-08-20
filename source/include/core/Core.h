#pragma once

#include "types.h"

namespace Playground {

template <typename T>
constexpr T AlignedForward(T v, T a)
{
    return (v + (a - 1)) & ~(a - 1);
}

template <class T>
constexpr void Swap(T& a, T& b)
{
    T c(std::move(a));
    a = std::move(b);
    b = std::move(c);
}

template <typename To, typename From>
constexpr To As(From f)
{
    return static_cast<To>(f);
}

template <typename T>
constexpr i64 SizeOf()
{
    return As<i64>(sizeof(T));
}

template <typename T, typename = std::enable_if<std::is_trivially_copyable_v<T>>::type>
T Min(T l, T r)
{
    if (r < l) {
        return r;
    }
    return l;
}

template <typename T, typename = std::enable_if<!std::is_trivially_copyable_v<T>>::type>
const T& Min(T const& l, T const& r)
{
    if (r < l) {
        return r;
    }
    return l;
}

template <typename T, typename = std::enable_if<std::is_trivially_copyable_v<T>>::type>
T Max(T l, T r)
{
    if (r > l) {
        return r;
    }
    return l;
}

template <typename T, typename = std::enable_if<!std::is_trivially_copyable_v<T>>::type>
const T& Max(T const& l, T const& r)
{
    if (r > l) {
        return r;
    }
    return l;
}

template <typename T, typename = std::enable_if<std::is_trivially_copyable_v<T>>::type>
T Clamp(T v, T l, T r)
{
    return Max(l, Min(v, r));
}

}