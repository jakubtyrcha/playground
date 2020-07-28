#pragma once

namespace Playground {
struct AABox2D {
    Vector2 vec_min;
    Vector2 vec_max;

    f32 Area() const;
    static AABox2D Xyxy(Vector2 xy0, Vector2 xy1);
};

struct AABox3D {
    Vector3 vec_min;
    Vector3 vec_max;
};

AABox2D GetAxisAlignedBoundingBox(Vector3 C, float r, float nearZ, Matrix4x4 P);
}