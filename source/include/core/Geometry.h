#pragma once

#include "types.h"
#include "array.h"

namespace Playground {

struct Aabb2D {
    Vector2 vec_min;
    Vector2 vec_max;

    Vector2 Min() const;
    Vector2 Max() const;
    Vector2 Span() const;

    Aabb2D Extended(Vector2 v) const;

    static Aabb2D Empty();
	static Aabb2D From(Vector2 a, Vector2 b);
};

struct Aabb3D {
    Vector3 vec_min;
    Vector3 vec_max;

    Vector3 Min() const;
    Vector3 Max() const;
    Vector3 Span() const;

    Aabb2D xz() const;

    Aabb3D Extended(Vector3 v) const;

    static Aabb3D Empty();
	static Aabb3D From(Vector3 a, Vector3 b);

    Array<Vector3> GetVertices() const;
};

struct Obb3D {
    Vector3 pos;
    Vector3 axis100;
    Vector3 axis010;
    Vector3 axis001;
    Vector3 half_size;

    Aabb3D GetAabb() const;
};

struct Sphere3D {
    Vector3 position;
    f32 radius;
};

Vector2 RandomPointInAnnulus(f32 r0, f32 r1, Vector2 random_pair);
Quaternion QuaternionRotationVectorToVector(Vector3 v0, Vector3 v1);
Vector3 Slerp(Vector3 start, Vector3 end, f32 f);
Vector3 SphericalToVector3(f32 polar, f32 azimuthal);
Vector3 SphericalToVector3(Vector2 c);

Matrix4 LookAtLh(Vector3 look_at, Vector3 eye, Vector3 up);
Matrix4 InverseLookAtLh(Vector3 look_at, Vector3 eye, Vector3 up);
Matrix4 PerspectiveFovLh(f32 aspect_ratio, f32 fov_y, f32 near_plane, f32 far_plane);
Matrix4 InversePerspectiveFovLh(f32 aspect_ratio, f32 fov_y, f32 near_plane, f32 far_plane);
Matrix4 OrthographicLh(f32 w, f32 h, f32 near_plane, f32 far_plane);
Matrix4 InverseOrthographicLh(f32 w, f32 h, f32 near_plane, f32 far_plane);

}