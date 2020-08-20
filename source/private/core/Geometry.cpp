#include "Pch.h"
#include "Geometry.h"

namespace Playground {

Vector2 AABox2D::Min() const
{
    return vec_min;
}

Vector2 AABox2D::Max() const
{
    return vec_max;
}

Vector2 AABox2D::Span() const
{
    return vec_max - vec_min;
}

AABox2D AABox2D::Extended(Vector2 v) const
{
    return From(Math::min(vec_min, v), Math::max(vec_max, v));
}

AABox2D AABox2D::Empty()
{
    return { .vec_min = Vector2 { Math::Constants<f32>::inf() }, .vec_max = Vector2 { -Math::Constants<f32>::inf() } };
}

AABox2D AABox2D::From(Vector2 a, Vector2 b)
{
    return { .vec_min = Math::min(a, b), .vec_max = Math::max(a, b) };
}

//

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

AABox2D AABox3D::xz() const
{
	return AABox2D::From(Min().xz(), Max().xz());
}

AABox3D AABox3D::Extended(Vector3 v) const
{
	return From(Math::min(vec_min, v), Math::max(vec_max, v));
}

AABox3D AABox3D::Empty()
{
    return { .vec_min = Vector3{ Math::Constants<f32>::inf() }, .vec_max = Vector3{ -Math::Constants<f32>::inf() } };
}

AABox3D AABox3D::From(Vector3 a, Vector3 b)
{
    return { .vec_min = Math::min(a, b), .vec_max = Math::max(a, b) };
}

Array<Vector3> AABox3D::GetVertices() const
{
	Vector3 v0 = Min();
    Vector3 span = Span();
    Vector3 e100 { span.x(), 0, 0 };
    Vector3 e010 { 0, span.y(), 0 };
    Vector3 e001 { 0, 0, span.z() };

	Array<Vector3> output;
	output.Reserve(8);
	output.PushBack(v0 + e100);
	output.PushBack(v0 + e100 + e001);
	output.PushBack(v0 + e001);
	output.PushBack(v0 + e010);
	output.PushBack(v0 + e010 + e100);
	output.PushBack(v0 + e010 + e100 + e001);
	output.PushBack(v0 + e010 + e001);
	return output;
}

}