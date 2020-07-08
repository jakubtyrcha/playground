#pragma once

template <typename T>
T AlignedForward(T v, T a)
{
    return (v + (a - 1)) & ~(a - 1);
}


