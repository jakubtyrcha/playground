#pragma once
#include "array.h"

namespace IO {
Containers::Array<char> GetFileContent(const wchar_t* path);
}