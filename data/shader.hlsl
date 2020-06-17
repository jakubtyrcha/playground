RWTexture2D<float4> TextureOut : register(u0);

[numthreads(8, 4, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    TextureOut[DTid.xy] = 1.f;
}