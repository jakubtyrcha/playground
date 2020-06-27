cbuffer VertexBuffer : register(b0) {
    float4x4 view_projection_matrix; 
};

struct VS_INPUT {
    float3 pos : POSITION;
    float4 col : COLOR0;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
};

PS_INPUT VsMain(VS_INPUT input) {
    PS_INPUT output;
    output.pos = mul(view_projection_matrix, float4(input.pos, 1));
    output.col = input.col;
    return output;
}
             
float4 PsMain(PS_INPUT input) : SV_TARGET {
    return input.col;
}