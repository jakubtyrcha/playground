#ifndef MATH_HLSLI
#define MATH_HLSLI

float2 TexcoordToClip(float2 uv) {
    return uv * float2( 2.0f, -2.0f ) + float2( -1.0f, 1.0f);
}

float3 Project(float4x4 m, float3 p) {
    float4 pp = mul(m, float4(p, 1));
    return pp.xyz / pp.w;
}

float Square(float x) {
	return x * x;
}

struct AABox2D {
    float2 min_max[2]; //[0] = min xy, [1] = max xy
};

AABox2D AABox2D_Xyxy(float x0, float y0, float x1, float y1) {
    AABox2D result;
    result.min_max[0].x = min(x0, x1);
    result.min_max[1].x = max(x0, x1);
    result.min_max[0].y = min(y0, y1);
    result.min_max[1].y = max(y0, y1);
    return result;
}

struct Ray {
    float3 origin;
    float3 direction;
};

struct Sphere {
    float3 center;
    float radius;
};

float RaySphereIntersection(Ray ray, Sphere sphere) {
    float3 m = ray.origin - sphere.center;
    float b = dot(m, ray.direction);
    float c = dot(m, m) - sphere.radius * sphere.radius;

    if(c > 0 && b > 0) {
        return -1;
    }

    float discr = b*b - c;
    if(discr < 0) {
        return -1;
    }

    float t = -b - sqrt(discr);

    return max(t, 0);
}

#endif