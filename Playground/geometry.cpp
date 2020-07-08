#include "pch.h"

#include "algorithms.h"
#include "geometry.h"

using namespace Algorithms;

namespace Geometry {
AABox2D AABox2D::Xyxy(Vector2 xy0, Vector2 xy1)
{
    return { .vec_min = { min(xy0.x(), xy1.x()), min(xy0.y(), xy1.y()) },
        .vec_max = { max(xy0.x(), xy1.x()), max(xy0.y(), xy1.y()) } };
}

f32 AABox2D::Area() const
{
    return (vec_max - vec_min).product();
}

Vector3 Project(Matrix4x4 P, Vector3 Q)
{
    Vector4 v = P * Vector4 { Q, 1.f };
    return v.xyz() / v.w();
}

// http://jcgt.org/published/0002/02/05/paper.pdf
struct AxisBounds {
    Vector3 L;
    Vector3 U;
};

f32 Square(f32 x)
{
    return x * x;
}

AxisBounds GetBoundForAxis(Vector3 a, Vector3 C, float r, float near_z)
{
    const Vector2 c(dot(a, C), C.z()); //C in the a-z frame
    Vector2 bounds[2]; //In the a-z reference frame
    const float t_squared = dot(c, c) - Square(r);
    const bool camera_inside_sphere = (t_squared <= 0);
    //(cos, sin) of angle theta between c and a tangent vector
    Vector2 v = camera_inside_sphere ? Vector2(0.0f, 0.0f) : Vector2(sqrt(t_squared), r) / c.length();

    //Does the near plane intersect the sphere?
    const bool clip_sphere = (c.y() - r <= near_z);
    //Square root of the discriminant; NaN (and unused)
    //if the camera is in the sphere
    float k = sqrt(Square(r) - Square(c.y() - near_z));
    for (int i = 0; i < 2; ++i) {
        if (!camera_inside_sphere) {
            bounds[i] = Matrix2x2 { Vector2 { v.x(), -v.y() }, Vector2 { v.y(), v.x() } } * c * v.x();
        }
        const bool clip_bound = camera_inside_sphere || (bounds[i].y() < near_z);
        if (clip_sphere && clip_bound) {
            bounds[i] = Vector2(c.x() + k, near_z);
        }
        //Set up for the lower bound
        v.y() = -v.y();
        k = -k;
    }

    AxisBounds result = { .L = { bounds[1].x() * a }, .U = { bounds[0].x() * a } };
    result.L.z() = bounds[1].y();
    result.U.z() = bounds[0].y();
    return result;
}

AABox2D GetAxisAlignedBoundingBox(Vector3 C, float r, float nearZ, Matrix4x4 P)
{
    const auto& [left, right] = GetBoundForAxis({ 1, 0, 0 }, C, r, nearZ);
    const auto& [bottom, top] = GetBoundForAxis({ 0, 1, 0 }, C, r, nearZ);

    return AABox2D::Xyxy({ Project(P, left).x(), Project(P, bottom).y() }, { Project(P, right).x(), Project(P, top).y() });
}
}