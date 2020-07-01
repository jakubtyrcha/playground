#pragma once

using namespace Magnum;

namespace Rendering {

	struct Viewport {
		Vector2i resolution;
		f32 fov_y;
		Vector3 camera_look_at;
		Vector3 camera_position;
		Vector3 camera_up;
		f32 near_plane;
		f32 far_plane;

		f32 GetAspectRatio() const;
		Vector3 GetLookToVector() const;
		Vector3 GetRightVector() const;
		void TranslateCamera(Vector3);

		Matrix4x4 view_matrix;
		Matrix4x4 inv_view_matrix;
		Matrix4x4 camera_offseted_view_matrix;
		Matrix4x4 camera_offseted_inv_view_matrix;
		Matrix4x4 projection_matrix;
		Matrix4x4 inv_projection_matrix;
		Matrix4x4 view_projection_matrix;
		Matrix4x4 inv_view_projection_matrix;
		Matrix4x4 camera_offseted_view_projection_matrix;
		Matrix4x4 inv_camera_offseted_view_projection_matrix;
		Matrix4x4 prev_view_matrix;
		Matrix4x4 prev_inv_view_matrix;
	};

	Matrix4 LookAtLh(Vector3 look_at, Vector3 eye, Vector3 up);
	Matrix4 InverseLookAtLh(Vector3 look_at, Vector3 eye, Vector3 up);
	Matrix4 PerspectiveFovLh(f32 aspect_ratio, f32 fov_y, f32 near_plane, f32 far_plane);
	Matrix4 InversePerspectiveFovLh(f32 aspect_ratio, f32 fov_y, f32 near_plane, f32 far_plane);
	Matrix4 PerspectiveFovLhReversedZ(f32 aspect_ratio, f32 fov_y, f32 near, f32 far);
	Matrix4 InversePerspectiveFovLhReversedZ(f32 aspect_ratio, f32 fov_y, f32 near, f32 far);

}