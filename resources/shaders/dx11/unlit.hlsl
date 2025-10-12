#include "common.hlsl"

PSInput VS_Main(VSInput input)
{
    PSInput output;
    output.clipPos = mul(float4(input.pos, 1.0f), mvp);
    return output; 
}

float4 PS_Main(PSInput input) : SV_TARGET
{   
    return float4(objectColor.x, objectColor.y, objectColor.z, 1.0);
}

