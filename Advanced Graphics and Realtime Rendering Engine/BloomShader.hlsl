Texture2D render : register(t0);
Texture2D renderHDR : register(t1);
RWTexture2D<float4> output : register(u0);

cbuffer Parameters : register(b0){
    bool firstPass;
    int strength;
    int2 resolution;
}

[numthreads(4, 4, 1)]
void CS( uint3 DTid : SV_DispatchThreadID )
{
	/***********************************************
	MARKING SCHEME: Bloom
	DESCRIPTION:	Guassian blur applied to the extracted bright areas of the render (via separate 
					HDR texture) and re-apply over regular render.
	REFERENCE:		https://learnopengl.com/Advanced-Lighting/Bloom
	***********************************************/
    const float weight[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };
    float4 result = renderHDR[DTid.xy];

    if (firstPass)
    {
        for (int i = 1; i < strength; ++i)
        {
            int2 uv1 = DTid.xy + int2(0, i);
            if(uv1.x >= resolution.x || uv1.y >= resolution.y)
                uv1 = resolution - int2(1, 1);
            result += renderHDR[uv1] * lerp(0.227027, 0.016216, (float) i / strength);

            int2 uv2 = DTid.xy - int2(0, i);
            if (uv2.x < 0 || uv2.y < 0)
                uv2 = int2(0, 0);
            result += renderHDR[uv2] * lerp(0.227027, 0.016216, (float)i / strength);
        }

        output[DTid.xy] = result / 2;
    }
    else
    {
        result = output[DTid.xy];
        for (int i = 1; i < strength; ++i)
        {
            int2 uv1 = DTid.xy + int2(i, 0);
            if (uv1.x >= resolution.x || uv1.y >= resolution.y)
                uv1 = resolution - int2(1, 1);
            result += output[uv1] * lerp(0.227027, 0.016216, (float)i / strength);

            int2 uv2 = DTid.xy - int2(i, 0);
            if (uv2.x < 0 || uv2.y < 0)
                uv2 = int2(0, 0);
            result += output[uv2] * lerp(0.227027, 0.016216, (float)i / strength);
        }

        output[DTid.xy] = render[DTid.xy] + output[DTid.xy] + (result / 2);
    }

}