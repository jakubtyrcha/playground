#include "frame.hlsli"

struct HeightfieldConstants {
    float3 aabb_min;
    int resolution_x;
    float3 aabb_max;
    float2 inv_resolution_xz;
    float3 directional_light_src;
};

cbuffer CB : register(b1) {
    HeightfieldConstants hf;
};

Texture2D<float> HeightFieldTexture : register(t0);
Texture2D<float> HorizonAngleTexture : register(t1);

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float3 wpos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float angle : TEXCOORD2;
};

PS_INPUT VsMain(uint VertexID: SV_VertexID) {
    uint vx = VertexID % hf.resolution_x;
    uint vy = VertexID / hf.resolution_x;    

    float3 span = hf.aabb_max - hf.aabb_min;
    float2 xz = span.xz * float2(vx, vy) * hf.inv_resolution_xz;
    float y = HeightFieldTexture.Load(uint3(vx, vy, 0)) * span.y;

    float y1 = HeightFieldTexture.Load(uint3(vx + 1, vy, 0)) * span.y;
    float y2 = HeightFieldTexture.Load(uint3(vx, vy + 1, 0)) * span.y;
    
    // TODO: better normals
    float3 N = -cross(float3(hf.inv_resolution_xz[0] * span.x, y1 - y, 0), float3(0, y2 - y, hf.inv_resolution_xz[1] * span.z));

    float4 pos = float4(hf.aabb_min + float3(xz[0], y, xz[1]), 1);

    // TODO: smoother shadows
    float3 L = hf.directional_light_src;
    float horizon_angle = HorizonAngleTexture.Load(uint3(vx, vy, 0));

    PS_INPUT output;
    output.pos = mul(frame.view_projection_matrix, pos);
    output.wpos = pos.xyz;
    output.normal = N;
    output.angle = horizon_angle;
    return output;
}

struct PS_OUTPUT {
    float4 colour : SV_TARGET0;
    float2 motion_vector : SV_TARGET1;
};

PS_OUTPUT PsMain(PS_INPUT input) {
    PS_OUTPUT output;

    float3 N = normalize(input.normal);
    float3 L = hf.directional_light_src;

    float shadow = L.y > input.angle;

    output.colour = dot(N, L) * 1.f * shadow;
    output.motion_vector = 0;

    float2 clip_space_xy = Frame_PixelPosToJitteredClip(input.pos.xy);
    float4 prev_clip_pos = mul(frame.prev_view_projection_matrix, float4(input.wpos, 1.f));
    output.motion_vector = ((prev_clip_pos.xy / prev_clip_pos.w) - clip_space_xy);

    return output;
}