Buffer<int> PageBuffer : register(t0);
Texture2D<float3> Position : register(t1);

RWTexture2D<uint> DepthTarget : register(u0);
RWTexture2D<float4> ColorTarget : register(u1);

struct FrameConstants {
    uint2 resolution;
    float4x4 view_projection_matrix;
};

cbuffer VertexBuffer : register(b0) {
    FrameConstants frame;
};

[numthreads(32, 1, 1)]
void ParticleDepthPassCs( uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint3 Gid : SV_GroupID ) {
    int page_index = PageBuffer[GTid.x];

    float4 pos = float4(Position[uint2(page_index, Gid.x)], 1.f);
    float4 clip_pos = mul(frame.view_projection_matrix, pos);
    clip_pos /= clip_pos.w;

    if(clip_pos.z < 0) {
        return;
    }
    if(any(clip_pos.xy) < -1 || any(clip_pos.xy) > 1) {
        return;
    }
    clip_pos.y *= -1;
    uint2 screen_texel = (clip_pos.xy * 0.5 + 0.5) * frame.resolution;

    InterlockedMax(DepthTarget[screen_texel], asuint(clip_pos.z));

    // remove
    ColorTarget[screen_texel] = float4(1, 0, 0, 1);
}