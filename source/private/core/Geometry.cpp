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

Vector3 Aabb3D::Center() const {
    return vec_min + Span() * 0.5f;
}

Aabb2D Aabb3D::xz() const
{
	return Aabb2D::From(Min().xz(), Max().xz());
}

Aabb3D Aabb3D::Union(Vector3 v) const
{
	return From(Math::min(vec_min, v), Math::max(vec_max, v));
}

Aabb3D Aabb3D::Union(Aabb3D const & other) const {
    return From(Math::min(vec_min, other.vec_min), Math::max(vec_max, other.vec_max));
}

f32 Aabb3D::Distance(Vector3 v) const {
    Vector3 clamped = Math::max(Math::min(v, vec_max), vec_min);
    return (clamped - v).length();
}

bool Aabb3D::Contains(Vector3 v) const {
    return (vec_min <= v && v <= vec_max).all();
}

bool Aabb3D::Contains(Aabb3D const & other) const {
    return Contains(other.vec_min) && Contains(other.vec_max);
}

f32 Aabb3D::Volume() const {
    Vector3 span = Span();
    return span.x() * span.y() * span.z();
}

f32 Aabb3D::Area() const {
    Vector3 span = Span();
    return 2.f * (span.x() * span.y() + span.y() * span.z() + span.z() * span.x());
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

bool Aabb3D::operator==(Aabb3D const& rhs) const {
    return vec_min == rhs.vec_min && vec_max == rhs.vec_max;
}

Aabb3D Aabb3D::Scaled(f32 f) const {
    Vector3 scaled_half_size = Span() * 0.5 * f;
    Vector3 center = Center();
    return { .vec_min = center - scaled_half_size, .vec_max = center + scaled_half_size };
}

//

Aabb3D Obb3D::GetAabb(Matrix4 const & transform) const
{
    Aabb3D bounds = Aabb3D::Empty();

    for (i32 a = 0; a < 2; a++) {
        for (i32 b = 0; b < 2; b++) {
            for (i32 c = 0; c < 2; c++) {
                f32 x = a * 2.f - 1.f;
                f32 y = b * 2.f - 1.f;
                f32 z = c * 2.f - 1.f;
                bounds = bounds.Union(transform.transformPoint(pos + axis100 * half_size.x() * x + axis010 * half_size.y() * y + axis001 * half_size.z() * z));
            }
        }
    }
    return bounds;
}

Obb3D Obb3D::UnitCube()
{
    return { 
        .pos = {}, 
        .axis100 = { 1.f, 0, 0 }, 
        .axis010 = { 0.f, 1.f, 0.f }, 
        .axis001 = { 0.f, 0.f, 1.f }, 
        .half_size = Vector3{ 0.5f } };
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