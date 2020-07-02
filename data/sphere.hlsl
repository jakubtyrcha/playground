struct FrameConstants {
    uint2 resolution;
    float2 inv_resolution;
    float4x4 view_projection_matrix;
    float4x4 inv_view_projection_matrix;
    float4x4 inv_view_matrix;
};

cbuffer CB : register(b0) {
    FrameConstants frame;
};

float3 ReadCameraPos() {
    return float3(frame.inv_view_matrix._41_42_43);
}

struct PS_INPUT {
    float4 pos : SV_POSITION;
};

PS_INPUT VsMain(uint VertexID: SV_VertexID) {
    PS_INPUT output;
    float2 uv = float2( (VertexID << 1) & 2, VertexID & 2 );
    output.pos = float4( uv * float2( 2.0f, -2.0f ) + float2( -1.0f, 1.0f), 0.0f, 1.0f );
    return output;
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
             
float4 PsMain(PS_INPUT input) : SV_TARGET {
    Sphere sphere;
    sphere.center = float3(0,0,0);
    sphere.radius = 1;

    float2 clip_space_xy = input.pos.xy * frame.inv_resolution * float2(2, -2) + float2(-1, 1);
    float4 ray_dir_wh = mul(frame.inv_view_projection_matrix, float4(clip_space_xy, 0, 1));
    float3 ray_dir = normalize(ray_dir_wh.xyz / ray_dir_wh.w);

    Ray screen_ray;
    screen_ray.origin = ReadCameraPos();
    screen_ray.direction = ray_dir;

    float t = RaySphereIntersection(screen_ray, sphere);

    return float4(clip_space_xy,0,0);
}