#include "Frame.hlsli"

struct VS_INPUT {
    float3 pos : POSITION;
    float4 col : COLOR0;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float3 wpos : POSITION;
    float4 col : COLOR0;
};

PS_INPUT VsMain(VS_INPUT input) {
    PS_INPUT output;
    output.pos = mul(frame.view_projection_matrix, float4(input.pos, 1));
    output.wpos = input.pos;
    output.col = input.col;
    return output;
}

struct PS_OUTPUT {
    float4 colour : SV_TARGET0;
    float2 motion_vector : SV_TARGET1;
};
             
PS_OUTPUT PsMain(PS_INPUT input) {
    PS_OUTPUT result;

    result.colour = input.col;

    float2 clip_space_xy = Frame_PixelPosToJitteredClip(input.pos.xy);

    float4 prev_clip_pos = mul(frame.prev_view_projection_matrix, float4(input.wpos, 1));

    // the problem with TAA for lines seems to be connected to rasterization rules: 
    // for a triangle pixel center has to be in the triangle, so we know where the
    // previous location would be just with reprojection; for line to find prev 
    // location we would have to apply rasterization rules with the prev view proj

    // idea: could figure out line clip location inside the pixel (middle point seems 
    // natural?) and reproject that instead of the pixel center

    result.motion_vector = 0;// ((prev_clip_pos.xy / prev_clip_pos.w) - clip_space_xy);

    return result;
}