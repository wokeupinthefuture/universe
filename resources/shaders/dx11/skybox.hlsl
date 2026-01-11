#include "variables.hlsl"

TextureCube skyboxTexture : register(t0);
SamplerState skyboxSampler : register(s0);

struct VSInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float4 clipPos : SV_POSITION;
    float3 uv : TEXCOORD;
};

PSInput VS_Main(VSInput input)
{
    PSInput output;

    output.clipPos = mul(float4(input.pos, 1.0f), v_mvp);
    output.clipPos.z = output.clipPos.w;

    output.uv = input.pos;

    return output; 
}

float4 PS_Main(PSInput input) : SV_TARGET
{   
    float3 uv = input.uv;
    return skyboxTexture.Sample(skyboxSampler, uv);
}
