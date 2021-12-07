RWTexture2D<float4> output : register(u0);

cbuffer ImageFilterConstantBuffer : register(b0) {
    int filterType;
    float filterIntensity;
    float time;
    float _padding;
    // 16 bytes
    float2 resolution;
    float2 __padding;
};

[numthreads(8, 8, 1)]
void CS( uint3 DTid : SV_DispatchThreadID )
{
    /***********************************************
	MARKING SCHEME: Additional Post Processing Effects
	DESCRIPTION:	4 post processing filters which can be changed and the intensity changed
					via the GUI.
	REFERENCE:		https://www.shadertoy.com/view/3sGGRz
	***********************************************/

    switch(filterType) {
    case 0: // Invert
        const float3 one = float3(1, 1, 1);
        const float4 invertCol = float4(one - output[DTid.xy].rgb, 1.0f);
        output[DTid.xy] = lerp(output[DTid.xy], invertCol, filterIntensity);
        break;
    case 1: // Greyscale
    	const float greyShade = (output[DTid.xy].r + output[DTid.xy].g + output[DTid.xy].b) / 3.0f;
        output[DTid.xy] = lerp(output[DTid.xy], float4(greyShade, greyShade, greyShade, 1.0f), filterIntensity);
        break;
    case 2: // Film Grain
        float noise = frac(sin(dot(DTid.xy + time, float2(12.9898, 78.233) * 2.0)) * 43758.5453);
        float3 grainCol = float3(noise, noise, noise);
        output[DTid.xy] = float4(output[DTid.xy].rgb - grainCol * filterIntensity, 1.0f);
        break;
    case 3: // Vignette
        float2 midCoord = resolution / 2.0f;
        float normDist = distance(midCoord, DTid.xy) / (resolution.x / 2.0f);
        output[DTid.xy] = output[DTid.xy] - (float4(normDist, normDist, normDist, 0.0f) * filterIntensity);
        break;
    }


    
}