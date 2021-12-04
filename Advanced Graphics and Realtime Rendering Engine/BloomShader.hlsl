Texture2D render : register(t0);
Texture2D renderHDR : register(t1);
RWTexture2D<float4> output : register(u0);

cbuffer Parameters : register(b0){
    float size;         // BLUR SIZE (Radius)
    float quality;      // BLUR QUALITY (Default 4.0 - More is better but slower)
    float directions;   // BLUR DIRECTIONS (Default 16.0 - More is better but slower)

    float _padding;
}

[numthreads(4, 4, 1)]
void CS( uint3 DTid : SV_DispatchThreadID )
{
	/***********************************************
	MARKING SCHEME: Bloom
	DESCRIPTION:	Guassian blur applied to the extracted emissive areas of the render (via separate 
					HDR texture) and re-apply result over regular render.
	REFERENCE:		https://learnopengl.com/Advanced-Lighting/Bloom
					https://www.shadertoy.com/view/Xltfzj
	***********************************************/

    const float PI2 = 6.28318530718; // Pi*2
    
    // Pixel colour
    float4 bloomCol = renderHDR[DTid.xy];
    
    // Blur calculations
    for (float d = 0.0; d < PI2; d += PI2 / directions)
    {
        for (float i = 1.0 / quality; i <= 1.0; i += 1.0 / quality)
        {
            bloomCol += renderHDR[DTid.xy + float2(cos(d), sin(d)) * size * i];
        }
    }
    
    // Output to screen
    bloomCol /= quality * directions - 15.0;
    output[DTid.xy] = render[DTid.xy] + bloomCol;
}