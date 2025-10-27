#include "common.hlsl"
#include "variables.hlsl"

PSInput VS_Main(VSInput input)
{
    PSInput output;
    output.clipPos = mul(float4(input.pos, 1.0f), v_mvp);
    output.worldPos = mul(float4(input.pos, 1.0f), v_world).xyz;
    output.worldNormal = mul(float4(input.normal, 0.f), v_world).xyz;
    output.uv = input.uv;
    return output; 
}

float4 PS_Main(PSInput input) : SV_TARGET
{ 
    float2 uv = input.uv;
    float4 textureSample = diffuseTexture.Sample(texSampler, input.uv);
    float4 color = textureSample * v_objectColor;
    if (all(textureSample == float4(0,0,0,0))) {
        color = float4(1, 0, 1, 1);
    }
    float3 normal = normalize(input.worldNormal);
    float3 lightVec = 0.0f;
    float attenuation = 1.f;

    if (v_lightType == 0)
    {
        lightVec = -v_lightDirection;
    }
    else if (v_lightType == 1)
    {
        lightVec = v_lightPosition - input.worldPos.xyz;
        float distance = length(lightVec);
        attenuation = 1.0f / (1.0f + 0.09f * distance + 0.032f * distance * distance);
    }   

    float diffuseFactor = max(dot(normal, normalize(lightVec)), 0.0);
    float4 diffuseColor = v_lightColor * diffuseFactor; 

    float4 ambientColor = color * 0.3;

    return float4(((ambientColor + (diffuseColor * attenuation)) * color).xyz, 1.0);
}

