Texture2D render : register(t0);
RWTexture2D<float4> output : register(u0);

cbuffer Parameters : register(b0)
{
    float size; // BLUR SIZE (Radius)
    float quality; // BLUR QUALITY 
    float directions; // BLUR DIRECTIONS 
    float farPlaneDepth;
    int2 resolution;
    float depth;
    float _padding;
}

[numthreads(8, 8, 1)]
void CS(uint3 DTid : SV_DispatchThreadID)
{
	/***********************************************
	MARKING SCHEME: 
	DESCRIPTION:	
	REFERENCE:      https://www.shadertoy.com/view/Xltfzj
	***********************************************/

    const float PI2 = 6.283185f; // Pi*2
    const float midPixelDepth = render[resolution / 2].w / farPlaneDepth;
    const float curPixelDepth = render[DTid.xy].w / farPlaneDepth;

    if(curPixelDepth == 1.0f) {
        output[DTid.xy] = float4(render[DTid.xy].rgb, 1.0f);
        return;
    }

    float targetDepth = ((curPixelDepth - midPixelDepth) / midPixelDepth) / depth;
    if (targetDepth < 0)
        targetDepth = abs(targetDepth) * 5.0f;
    
    // Pixel colour
    float3 blurCol = render[DTid.xy].rgb;
    
    // Blur calculations
    for (float d = 0.0; d < PI2; d += PI2 / directions)
    {
        for (float i = 1.0 / quality; i <= 1.0; i += 1.0 / quality)
        {
            float2 uv = DTid.xy + float2(cos(d), sin(d)) * size * i;
            blurCol += render[uv].rgb;
        }
    }
    
    // Output to screen
    blurCol /= quality * directions - 15.0;
    output[DTid.xy] = float4(lerp(render[DTid.xy].rgb, blurCol, targetDepth), 1.0f);
}