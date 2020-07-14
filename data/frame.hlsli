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

float3 GetCameraWPos() {
    return frame.inv_view_matrix._14_24_34;
}

