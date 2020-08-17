#include "Pch.h"
#include "Algorithms.h"

namespace Playground {

f32 Frac(f32 x)
{
    f32 _;
    return modf(x, &_);
}

}