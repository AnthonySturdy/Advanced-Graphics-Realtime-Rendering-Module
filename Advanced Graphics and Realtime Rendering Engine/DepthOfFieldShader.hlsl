Texture2D render : register(t0);
RWTexture2D<float4> output : register(u0);

cbuffer Parameters : register(b0)
{
    float size; // BLUR SIZE (Radius)
    float quality; // BLUR QUALITY 
    float directions; // BLUR DIRECTIONS 
    float farPlaneDepth;
    int2 resolution;
    float falloff;
    float _padding;
}

[numthreads(8, 8, 1)]
void CS(uint3 DTid : SV_DispatchThreadID)
{
	/***********************************************
	MARKING SCHEME: Depth of Field 
	DESCRIPTION:	Gaussian blur applied and strength determined by
					scene depth and what the camera is pointing at
	REFERENCE:      https://xorshaders.weebly.com/tutorials/blur-shaders-5-part-2
	***********************************************/

    const float PI2 = 6.283185f; // Pi*2
    const float midPixelDepth = render[resolution / 2].w / farPlaneDepth;   // Depth of pixel at centre of screen
    const float curPixelDepth = render[DTid.xy].w / farPlaneDepth;  // Depth of current pixel

    float targetDepth = ((curPixelDepth - midPixelDepth) / midPixelDepth) / falloff;  // Depth diff between mid and cur. pixel
    if (targetDepth < 0)    // If foreground should be blurred
        targetDepth = abs(targetDepth) * 4.0f;
    targetDepth = saturate(targetDepth);
    
    // Blur algorithm
    float3 blurCol = render[DTid.xy].rgb;
    for (float d = 0.0; d < PI2; d += PI2 / directions)
    {
        for (float i = 1.0 / quality; i <= 1.0; i += 1.0 / quality)
        {
            const float2 uv = DTid.xy + float2(cos(d), sin(d)) * size * i;
            blurCol += render[uv].rgb;
        }
    }
    
    // Output to screen
    blurCol /= quality * directions - 15.0;
    output[DTid.xy] = float4(lerp(render[DTid.xy].rgb, blurCol, targetDepth), 1.0f);    // Interpolate based on depth calculation
}