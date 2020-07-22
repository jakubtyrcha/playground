#include "frame.hlsli"

Texture2D<float4> ColourTexture : register(t0);
Texture2D<float> DepthTexture : register(t1);
Texture2D<float4> PrevColourTexture : register(t2);
Texture2D<float2> MotionVectorTexture : register(t3);

#define TAA_TAP_CROSS 0
#define TAA_TAP_BOX 1

#define TAA_TAP TAA_TAP_BOX

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

PS_OUTPUT OutputColour(float4 colour) {
    PS_OUTPUT result;
    result.colour_copy = colour;
    result.colour = colour;
    return result;
}

PS_OUTPUT OutputDebugColour(float4 colour, float4 value) {
    PS_OUTPUT result;
    result.colour_copy = colour;
    result.colour = value;
    return result;
}

PS_OUTPUT PSMain(PS_INPUT input) {
    float depth = DepthTexture[input.pos.xy];
    float4 colour = ColourTexture[input.pos.xy];
    PixelInfo pixel_info = GetPixelInfo(input.pos, depth);

    float2 motion_vector = MotionVectorTexture[input.pos.xy];
    float2 prev_pixel_coord = input.pos.xy + motion_vector * float2(0.5, -0.5) * frame.resolution;

    bool history_off_screen = any(prev_pixel_coord.xy < 0) || any(frame.resolution < prev_pixel_coord.xy);
    if(history_off_screen) {
        return OutputColour(colour);
    }

    float4 prev_colour = PrevColourTexture[prev_pixel_coord.xy];

    float3 colour_aabb_min = colour.xyz;
    float3 colour_aabb_max = colour.xyz;

#if TAA_TAP == TAA_TAP_CROSS
    const float2 neightbour_offset[4] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    for(int i=0; i<4; i++) {
        float3 neighbour_colour = ColourTexture[input.pos.xy + neightbour_offset[i]].xyz;
        colour_aabb_min = min(neighbour_colour, colour_aabb_min);
        colour_aabb_max = max(neighbour_colour, colour_aabb_max);
    }
#elif TAA_TAP == TAA_TAP_BOX
    const float2 neightbour_offset[8] = {{-1, 1}, {0, 1}, {1, 1}, {-1, 0}, {1, 0}, {-1, -1}, {0, -1}, {1, -1} };
    for(int i=0; i<8; i++) {
        float3 neighbour_colour = ColourTexture[input.pos.xy + neightbour_offset[i]].xyz;
        colour_aabb_min = min(neighbour_colour, colour_aabb_min);
        colour_aabb_max = max(neighbour_colour, colour_aabb_max);
    }
#endif

    if(any(prev_colour.xyz < colour_aabb_min) || any(colour_aabb_max < prev_colour.xyz)) {
        return OutputColour(colour);
        //return OutputDebugColour(colour, float4(0, 1, 0, 0));
    }

    float rate = frame.taa_history_decay;

    return OutputColour(colour * (1 - frame.taa_history_decay) + prev_colour * frame.taa_history_decay);
}