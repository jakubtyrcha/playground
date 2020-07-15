Texture2D<float4> ColourTexture : register(t0);

struct PS_INPUT {
    float4 pos : SV_POSITION;
};

float2 TexcoordToClip(float2 uv) {
    return uv * float2( 2.0f, -2.0f ) + float2( -1.0f, 1.0f);
}

PS_INPUT VSMain(uint VertexID: SV_VertexID) {
    PS_INPUT output;
    float2 uv = float2( (VertexID << 1) & 2, VertexID & 2 );
    output.pos = float4( TexcoordToClip(uv), 0.0f, 1.0f );
    return output;
}

float4 PSMain(PS_INPUT input) : SV_TARGET {
    return ColourTexture[input.pos.xy];
}