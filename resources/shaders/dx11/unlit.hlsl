#include "common.hlsl"
#include "variables.hlsl"

PSInput VS_Main(VSInput input)
{
    PSInput output;
    output.clipPos = mul(float4(input.pos, 1.0f), v_mvp);
    output.uv = input.uv;
    return output; 
}

float4 PS_Main(PSInput input) : SV_TARGET
{   
    float2 uv = input.uv;
    float4 textureColor = diffuseTexture.Sample(texSampler, uv);
    if (all(textureColor == float4(0, 0, 0, 0)))
        return v_objectColor;
    return textureColor * v_objectColor;
}

