#pragma once

#include "types.h"
#include <debug_assert/debug_assert.hpp>
#include <stdio.h>
#include <type_traits>

namespace Playground {
struct containers_module
    : debug_assert::default_handler, // use the default handler
      debug_assert::set_level<-1> // level -1, i.e. all assertions, 0 would mean none, 1 would be level 1, 2 level 2 or lower,...
{
};
}