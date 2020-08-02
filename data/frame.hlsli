#include "Math.hlsli"

struct FrameConstants {
    uint2 resolution;
    float2 inv_resolution;
    float2 near_far_planes;
    float2 clipspace_jitter;
    float4x4 view_matrix;
    float4x4 projection_matrix;
    float4x4 view_projection_matrix;
    float4x4 inv_view_projection_matrix;
    float4x4 inv_view_matrix;
    float4x4 prev_view_projection_matrix;
    float taa_history_decay;
};

cbuffer CB : register(b0) {
    FrameConstants frame;
};

float3 Frame_GetCameraWPos() {
    return frame.inv_view_matrix._14_24_34;
}

float Frame_LinearizeDepth(float zbuffer_depth) {
    return frame.projection_matrix._33 + frame.projection_matrix._34 / zbuffer_depth;
}

float2 Frame_ClipToPixel(float2 cs) {
    float2 ts = cs * float2(0.5, -0.5) + 0.5;
    return ts * frame.resolution;
}

float2 Frame_PixelPosToClip(float2 pos) {
    return TexcoordToClip(pos * frame.inv_resolution);
}

float2 Frame_PixelPosToJitteredClip(float2 pos) {
    return TexcoordToClip(pos * frame.inv_resolution) - frame.clipspace_jitter;
}