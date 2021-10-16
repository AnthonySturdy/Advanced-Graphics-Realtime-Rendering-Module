cbuffer ConstantBuffer : register(b0) {
    matrix World;
    matrix View;
    matrix Projection;
    float4 vOutputColor;
}

struct VS_INPUT {
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT {
    float4 Pos : SV_POSITION;
    float4 worldPos : POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
};

PS_INPUT VS(VS_INPUT input) {
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul(input.Pos, World);
    output.worldPos = output.Pos;
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);

    // multiply the normal by the world transform (to go from model space to world space)
    output.Norm = mul(float4(input.Norm, 1), World).xyz;

    output.Tex = input.Tex;

    return output;
}