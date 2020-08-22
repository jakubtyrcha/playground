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

Vector2 RandomPointInAnnulus(f32 r0, f32 r1, Vector2 random_pair);
Quaternion QuaternionRotationVectorToVector(Vector3 v0, Vector3 v1);

Matrix4 LookAtLh(Vector3 look_at, Vector3 eye, Vector3 up);
Matrix4 InverseLookAtLh(Vector3 look_at, Vector3 eye, Vector3 up);
Matrix4 PerspectiveFovLh(f32 aspect_ratio, f32 fov_y, f32 near_plane, f32 far_plane);
Matrix4 InversePerspectiveFovLh(f32 aspect_ratio, f32 fov_y, f32 near_plane, f32 far_plane);
Matrix4 OrthographicLh(f32 w, f32 h, f32 near_plane, f32 far_plane);
Matrix4 InverseOrthographicLh(f32 w, f32 h, f32 near_plane, f32 far_plane);

}