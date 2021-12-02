Texture2D render : register(t0);
Texture2D renderHDR : register(t1);
RWTexture2D<float4> output : register(u0);

[numthreads(8, 8, 1)]
void CS( uint3 DTid : SV_DispatchThreadID )
{
    output[DTid.xy] = render[DTid.xy] * (DTid.x / 500.0f);
}