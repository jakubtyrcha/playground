#pragma once

#include "Array.h"

namespace Playground {

struct String {
    Array<wchar_t> data_;

    String();
    String(const wchar_t*);
    operator const wchar_t*() const;
};

}