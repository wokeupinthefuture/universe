#include "common.hlsl"

PSInput VS_Main(VSInput input)
{
    PSInput output;
    output.pos = mul(float4(input.pos, 1.0f), mvp);
    output.worldNormal = mul(float4(input.normal, 0.f), world).xyz;
    return output; 
}

float4 PS_Main(PSInput input) : SV_TARGET
{ 
    float3 normal = normalize(input.worldNormal);
    float3 lightVec = 0.0f;
    float attenuation = 1.0f;   

    if (lightType == 0)
    {
        lightVec = -lightDirection;
    }
    else if (lightType == 1)
    {
        lightVec = float3(-3, 0, 0) - input.pos.xyz;
        float distance = 1;//length(lightVec);
        //attenuation = 1.0f / (1.0f + 0.09f * distance + 0.032f * distance * distance);
    }   

    float diffuseFactor = max(dot(normal, lightVec), 0.0);
    float4 diffuseColor = lightColor * diffuseFactor; 

    float4 ambientColor = objectColor * 0.1;

    return float4(((ambientColor + (diffuseColor * attenuation)) * objectColor).xyz, 1.0);
}

