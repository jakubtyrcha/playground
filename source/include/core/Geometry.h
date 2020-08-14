#pragma once

#include "types.h"
#include "array.h"

namespace Playground {

struct AABox3D {
    Vector3 vec_min;
    Vector3 vec_max;

    Vector3 Min() const;
    Vector3 Max() const;
    Vector3 Span() const;

    AABox3D Extended(Vector3 v) const;

    static AABox3D Empty();
	static AABox3D From(Vector3 a, Vector3 b);

    Array<Vector3> GetVertices() const;
};

}