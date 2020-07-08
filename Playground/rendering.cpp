#include "pch.h"

#include "rendering.h"

namespace Rendering {

f32 Viewport::GetAspectRatio() const
{
    return resolution.x() / static_cast<f32>(resolution.y());
}

Vector3 Viewport::GetLookToVector() const
{
    return (camera_look_at - camera_position).normalized();
}

Vector3 Viewport::GetRightVector() const
{
    Vector3 z = (camera_look_at - camera_position).normalized();
    return Math::cross(camera_up, z).normalized();
}

void Viewport::TranslateCamera(Vector3 v)
{
    camera_position += v;
    camera_look_at += v;
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

}