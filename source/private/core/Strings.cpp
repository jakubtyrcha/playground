#include "pch.h"

#include "Strings.h"

namespace Playground {

String::String()
{
    data_.Resize(1);
}

String::String(const wchar_t* str)
{
    i64 len = wcslen(str);
    data_.Reserve(len + 1);
    data_.Append(str, len);
    data_.PushBack(0);
}

String::operator const wchar_t*() const
{
    return data_.Data();
}

}