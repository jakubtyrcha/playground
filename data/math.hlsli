float2 TexcoordToClip(float2 uv) {
    return uv * float2( 2.0f, -2.0f ) + float2( -1.0f, 1.0f);
}