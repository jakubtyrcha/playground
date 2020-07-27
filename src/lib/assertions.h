#pragma once

// TODO: rename assert macros to avoid collision with other APIs

#include <debug_assert/debug_assert.hpp>

struct default_module
    : debug_assert::default_handler, // use the default handler
      debug_assert::set_level<-1> // level -1, i.e. all assertions, 0 would mean none, 1 would be level 1, 2 level 2 or lower,...
{
};

// todo: fix redef warning
#define assert(x) DEBUG_ASSERT(static_cast<bool>(x), default_module {});
#define verify(x)                                     \
    {                                                 \
        bool __verify_ok = (x);                       \
        DEBUG_ASSERT(__verify_ok, default_module {}); \
    }