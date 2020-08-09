#include "Pch.h"
#include "Geometry.h"

namespace Playground {

Vector3 AABox3D::Min() const
{
	return vec_min;
}

Vector3 AABox3D::Max() const
{
	return vec_max;
}

Vector3 AABox3D::Span() const
{
	return vec_max - vec_min;
}

AABox3D AABox3D::From(Vector3 a, Vector3 b)
{
    return { .vec_min = Math::min(a, b), .vec_max = Math::max(a, b) };
}
}