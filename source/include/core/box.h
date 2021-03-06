#pragma once

#include "types.h"
#include <magnum/CorradePointer.h>

namespace Playground {

template <typename T>
using Box = Corrade::Containers::Pointer<T>;

template <typename T, typename... Args>
Box<T> MakeBox(Args&&... args)
{
    return Box<T> { new T { std::forward<Args>(args)... } };
}

template <typename T>
struct MoveableNonCopyable {
    MoveableNonCopyable(const MoveableNonCopyable&) = delete;
    MoveableNonCopyable& operator=(const MoveableNonCopyable&) = delete;

    MoveableNonCopyable(MoveableNonCopyable&&) = default;
    MoveableNonCopyable& operator=(MoveableNonCopyable&&) = default;

protected:
    MoveableNonCopyable() = default;
    ~MoveableNonCopyable() = default;
};

template <typename T>
struct Pinned {
    Pinned(const Pinned&) = delete;
    Pinned& operator=(const Pinned&) = delete;

    Pinned(Pinned&&) = delete;
    Pinned& operator=(Pinned&&) = delete;

protected:
    Pinned() = default;
    ~Pinned() = default;
};

}