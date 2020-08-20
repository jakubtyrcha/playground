#pragma once

#include "types.h"
#include "array.h"

namespace Playground {

struct AABox2D {
    Vector2 vec_min;
    Vector2 vec_max;

    Vector2 Min() const;
    Vector2 Max() const;
    Vector2 Span() const;

    AABox2D Extended(Vector2 v) const;

    static AABox2D Empty();
	static AABox2D From(Vector2 a, Vector2 b);
};

struct AABox3D {
    Vector3 vec_min;
    Vector3 vec_max;

    Vector3 Min() const;
    Vector3 Max() const;
    Vector3 Span() const;

    AABox2D xz() const;

    AABox3D Extended(Vector3 v) const;

    static AABox3D Empty();
	static AABox3D From(Vector3 a, Vector3 b);

    Array<Vector3> GetVertices() const;
};

}