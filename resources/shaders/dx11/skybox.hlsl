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

    matrix view = v_mvp;
    view[3][0] = 0.0;
    view[3][1] = 0.0;
    view[3][2] = 0.0;

    output.clipPos = mul(float4(input.pos, 1.0f), view);
    output.clipPos.z = output.clipPos.w;

    output.uv = input.pos;

    return output; 
}

float4 PS_Main(PSInput input) : SV_TARGET
{   
    float3 uv = input.uv;
    return skyboxTexture.Sample(skyboxSampler, uv);
}
