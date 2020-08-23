#include "Pch.h"
#include "Geometry.h"

namespace Playground {

Vector2 Aabb2D::Min() const
{
    return vec_min;
}

Vector2 Aabb2D::Max() const
{
    return vec_max;
}

Vector2 Aabb2D::Span() const
{
    return vec_max - vec_min;
}

Aabb2D Aabb2D::Extended(Vector2 v) const
{
    return From(Math::min(vec_min, v), Math::max(vec_max, v));
}

Aabb2D Aabb2D::Empty()
{
    return { .vec_min = Vector2 { Math::Constants<f32>::inf() }, .vec_max = Vector2 { -Math::Constants<f32>::inf() } };
}

Aabb2D Aabb2D::From(Vector2 a, Vector2 b)
{
    return { .vec_min = Math::min(a, b), .vec_max = Math::max(a, b) };
}

//

Vector3 Aabb3D::Min() const
{
	return vec_min;
}

Vector3 Aabb3D::Max() const
{
	return vec_max;
}

Vector3 Aabb3D::Span() const
{
	return vec_max - vec_min;
}

Aabb2D Aabb3D::xz() const
{
	return Aabb2D::From(Min().xz(), Max().xz());
}

Aabb3D Aabb3D::Extended(Vector3 v) const
{
	return From(Math::min(vec_min, v), Math::max(vec_max, v));
}

Aabb3D Aabb3D::Empty()
{
    return { .vec_min = Vector3{ Math::Constants<f32>::inf() }, .vec_max = Vector3{ -Math::Constants<f32>::inf() } };
}

Aabb3D Aabb3D::From(Vector3 a, Vector3 b)
{
    return { .vec_min = Math::min(a, b), .vec_max = Math::max(a, b) };
}

Array<Vector3> Aabb3D::GetVertices() const
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

//

Aabb3D Obb3D::GetAabb() const
{
    Aabb3D bounds = Aabb3D::Empty();
    bounds = bounds.Extended(pos - axis100 * half_size.x());
    bounds = bounds.Extended(pos + axis100 * half_size.x());
    bounds = bounds.Extended(pos - axis010 * half_size.y());
    bounds = bounds.Extended(pos + axis010 * half_size.y());
    bounds = bounds.Extended(pos - axis001 * half_size.z());
    bounds = bounds.Extended(pos + axis001 * half_size.z());
    return bounds;
}

//

Vector2 RandomPointInAnnulus(f32 r0, f32 r1, Vector2 random_pair)
{
    plgr_assert(r0 < r1);

    f32 theta = 2.f * Magnum::Math::Constants<f32>::pi() * random_pair.x();
    f32 d = sqrtf(random_pair.y() * (r1 * r1 - r0 * r0) + r0 * r0);

    return d * Vector2 { cosf(theta), sinf(theta) };
}

Quaternion QuaternionRotationVectorToVector(Vector3 v0, Vector3 v1)
{
    return Quaternion{Math::cross(v0, v1), sqrtf(v0.dot() * v1.dot()) + Math::dot(v0, v1)}.normalized();
}

Matrix4 LookAtLh(Vector3 look_at, Vector3 eye, Vector3 up)
{
    Vector3 z = (look_at - eye).normalized();
    Vector3 x = Math::cross(up, z).normalized();
    Vector3 y = Math::cross(z, x);

    return Matrix4 { Vector4 { x, -Math::dot(x, eye) }, Vector4 { y, -Math::dot(y, eye) }, Vector4 { z, -Math::dot(z, eye) }, Vector4 { 0, 0, 0, 1 } }.transposed();
}

Matrix4 InverseLookAtLh(Vector3 look_at, Vector3 eye, Vector3 up)
{
    Matrix4 view = LookAtLh(look_at, eye, up);
    return Matrix4::from(view.rotation().transposed(), eye);
}

Matrix4 PerspectiveFovLh(f32 aspect_ratio, f32 fov_y, f32 near_plane, f32 far_plane)
{
    f32 yscale = 1.f / tanf(fov_y * 0.5f);
    f32 xscale = yscale / aspect_ratio;

    return {
        Vector4 { xscale, 0, 0, 0 },
        Vector4 { 0, yscale, 0, 0 },
        Vector4 { 0, 0, far_plane / (far_plane - near_plane), 1 },
        Vector4 { 0, 0, -near_plane * far_plane / (far_plane - near_plane), 0 }
    };
}

Matrix4 InversePerspectiveFovLh(f32 aspect_ratio, f32 fov_y, f32 near_plane, f32 far_plane)
{
    f32 yscale = tanf(fov_y * 0.5f);
    f32 xscale = yscale * aspect_ratio;
    f32 ab = -1.f / near_plane;

    return {
        Vector4 { xscale, 0, 0, 0 },
        Vector4 { 0, yscale, 0, 0 },
        Vector4 { 0, 0, 0, ab },
        Vector4 { 0, 0, 1, -ab }
    };
}

Matrix4 OrthographicLh(f32 w, f32 h, f32 near_plane, f32 far_plane)
{
    return {
        Vector4 { 2.f / w, 0, 0, 0 },
        Vector4 { 0, 2.f / h, 0, 0 },
        Vector4 { 0, 0, 1.f / (far_plane - near_plane), 0 },
        Vector4 { 0, 0, near_plane / (near_plane - far_plane), 1 }
    };
}

Matrix4 InverseOrthographicLh(f32 w, f32 h, f32 near_plane, f32 far_plane)
{
    // TODO: test correctness
    return {
        Vector4 { w / 2.f, 0, 0, 0 },
        Vector4 { 0, h / 2.f, 0, 0 },
        Vector4 { 0, 0, far_plane - near_plane, 0 },
        Vector4 { 0, 0, near_plane, 1 }
    };
}

}