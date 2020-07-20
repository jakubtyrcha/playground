Texture2D<float> DepthTexture : register(t0);
Texture2D<float2> MotionVectorTexture : register(t1);

#include "frame.hlsli"
#include "math.hlsli"

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float4 colour : COLOR;
};

PS_INPUT VSMain(uint VertexID: SV_VertexID, uint InstanceID : SV_InstanceID) {
    PS_INPUT output;

    const uint RESOLUTION = 32;
    uint x_trackers = (frame.resolution.x + RESOLUTION - 1) / RESOLUTION;

    uint x = InstanceID % x_trackers;
    uint y = InstanceID / x_trackers;

    float2 pos_0 = TexcoordToClip(float2(x * RESOLUTION * frame.inv_resolution.x, y * RESOLUTION * frame.inv_resolution.y));

    output.pos = float4(pos_0, 0.0f, 1.0f );
    output.colour = 1;
    
    if(VertexID == 1) {
        float2 motion_vector = MotionVectorTexture[float2(x, y) * RESOLUTION];
        output.pos.xy += motion_vector;
        output.colour = 0.01f;
    }
    
    return output;
}

float4 PSMain(PS_INPUT input) : SV_TARGET {
    return input.colour;
}