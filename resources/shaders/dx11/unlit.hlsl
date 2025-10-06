#include "common.hlsl"

PSInput VS_Main(VSInput input)
{
    PSInput output;
    output.pos = mul(float4(input.pos, 1.0f), ul_mvp);
    return output; 
}

float4 PS_Main(PSInput input) : SV_TARGET
{   
    return float4(ul_objectColor.x, ul_objectColor.y, ul_objectColor.z, 1.0);
}

