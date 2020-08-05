#pragma once

#include <stdint.h>

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;

#include "assertions.h"

// TODO: enable asserts in release
#ifndef CORRADE_ASSERT
#define CORRADE_ASSERT(condition, message, returnValue) plgr_assert(condition)
#endif

#include <magnum/MagnumMath.hpp>
using namespace Magnum;

#include <magnum/CorradeOptional.h>

namespace Playground {
template <typename T>
using Optional = Corrade::Containers::Optional<T>;
constexpr Corrade::Containers::NullOptT NullOpt = Corrade::Containers::NullOpt;
}