#include "frame.hlsli"

Texture2D<float4> ColourTexture : register(t0);
Texture2D<float> DepthTexture : register(t1);
Texture2D<float4> PrevColourTexture : register(t2);
Texture2D<float2> MotionVectorTexture : register(t3);

float2 TexcoordToClip(float2 uv) {
    return uv * float2( 2.0f, -2.0f ) + float2( -1.0f, 1.0f);
}

struct PS_INPUT {
    float4 pos : SV_POSITION;
};

PS_INPUT VSMain(uint VertexID: SV_VertexID) {
    PS_INPUT output;
    float2 uv = float2( (VertexID << 1) & 2, VertexID & 2 );
    output.pos = float4( TexcoordToClip(uv), 0.0f, 1.0f );
    return output;
}

struct PixelInfo {
    float2 cs_coord;
    float3 V;
    float3 ws_position;
};

PixelInfo GetPixelInfo(float4 postproj_pos, float depth) {
    PixelInfo output;
    output.cs_coord = postproj_pos.xy * frame.inv_resolution * float2(2, -2) + float2(-1, 1) + frame.clipspace_jitter;
    float4 ray_dir_wh = mul(frame.inv_view_projection_matrix, float4(output.cs_coord, 1, 1));
    output.V = -normalize(ray_dir_wh.xyz);
    output.ws_position = Frame_GetCameraWPos() + ray_dir_wh.xyz / ray_dir_wh.z * Frame_LinearizeDepth(depth);
    return output;
}

struct PS_OUTPUT {
    float4 colour_copy : SV_TARGET0;
    float4 colour : SV_TARGET1;
};

PS_OUTPUT PSMain(PS_INPUT input) {
    PS_OUTPUT result;

    float depth = DepthTexture[input.pos.xy];
    float4 colour = ColourTexture[input.pos.xy];
    PixelInfo pixel_info = GetPixelInfo(input.pos, depth);

    float2 motion_vector = MotionVectorTexture[input.pos.xy];
    float2 prev_pixel_coord = input.pos.xy + motion_vector * float2(0.5, -0.5) * frame.resolution;

    /*
    bool history_off_screen = any(prev_postproj_pos.xy < -1) || any(1 < prev_postproj_pos.xy);
    if(history_off_screen) {
        return colour;
    }

    float4 prev_colour = PrevColourTexture[ClipToPixel(prev_postproj_pos.xy)];
    */
    float4 prev_colour = PrevColourTexture[prev_pixel_coord.xy];

    //return all(colour.xyz == prev_colour.xyz);

    float rate = frame.taa_history_decay;

    result.colour_copy = colour * (1 - frame.taa_history_decay) + prev_colour * frame.taa_history_decay;
    result.colour = result.colour_copy;

    result.colour = dot(prev_colour - colour, prev_colour - colour);
    result.colour = prev_colour;

    return result;
}