#ifndef SPHERE_BOUNDS_HLSLI
#define SPHERE_BOUNDS_HLSLI

#include "math.hlsli"

struct AxisBounds {
	float3 L;
	float3 U;
};

AxisBounds GetBoundForAxis(float3 a, float3 C, float r, float near_z) {
	float2 c = float2(dot(a, C), C.z); //C in the a-z frame
	float2 bounds[2]; //In the a-z reference frame
	const float t_squared = dot(c, c) - Square(r);
	const bool camera_inside_sphere = (t_squared <= 0); 
	//(cos, sin) of angle theta between c and a tangent vector
	float2 v = camera_inside_sphere ?
		float2(0.0f, 0.0f) :
		float2(sqrt(t_squared), r) / length(c);

	//Does the near plane intersect the sphere?
	const bool clip_sphere = (c.y - r <= near_z);
	//Square root of the discriminant; NaN (and unused)
	//if the camera is in the sphere
	float k = sqrt(Square(r) - Square(c.y - near_z));
	for (int i = 0; i < 2; ++i) {
		if (! camera_inside_sphere) {
			bounds[i] = mul(float2x2( v.x, v.y, -v.y, v.x ), c) * v.x;
		}
		const bool clip_bound = camera_inside_sphere || (bounds[i].y < near_z);
		if (clip_sphere && clip_bound) {
			bounds[i] = float2(c.x + k, near_z);
		}
		//Set up for the lower bound
		v.y = -v.y;  k = -k;
	}
	AxisBounds result = { { bounds[1].x * a }, { bounds[0].x * a }};
	result.L.z = bounds[1].y;
	result.U.z = bounds[0].y;
	return result;
}

#endif