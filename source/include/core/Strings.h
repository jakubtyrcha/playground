#pragma once

#include "Array.h"

namespace Playground {

struct String {
    Array<char> data_;

    String();
    String(const char*);

    operator const char*() const;
};

struct WString {
    Array<wchar_t> data_;

    WString();
    WString(const wchar_t*);

    operator const wchar_t*() const;
};

}