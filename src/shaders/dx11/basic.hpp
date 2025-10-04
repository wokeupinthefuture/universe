#pragma once

namespace Shaders::Basic
{

struct Variables
{
    float world[4][4];
    float mvp[4][4];
    float objectColor[4];
    float lightColor[4];
    float lightDirection[3];
    float _padding0;
    float lightPosition[3];
    float time;
    int lightType;
    float _padding1[3];
};
static_assert(sizeof(Variables) % 16 == 0, "Constant buffer size must be a multiple of 16 bytes");

static constexpr Variables DEFAULT_VARIABLES = {
    .world = {},
    .mvp = {},
    .objectColor = {0.5f, 0.f, 0.5f, 1.f},
    .lightColor = {1.f, 1.f, 1.f, 1.f},
    .lightDirection = {0.f, 0.f, 1.f},
    ._padding0 = {},
    .lightPosition = {},
    .time = 0.f,
    .lightType = 0,
    ._padding1 = {},
};

static constexpr auto vs = R"(
    cbuffer ConstantBuffer : register(b0)
    {
        float4x4 world;
        float4x4 mvp;
        float4 objectColor;
        float4 lightColor;
        float3 lightDirection;
        float3 lightPosition;
        float time;
        int lightType;
    };

    struct VSInput
    {
        float3 pos : POSITION;
        float3 normal : NORMAL;
    };

    struct VSOutput
    {
        float4 pos : SV_POSITION;
        float3 worldNormal : NORMAL;
    };

    VSOutput main(VSInput input)
    {
        VSOutput output;
        output.pos = mul(float4(input.pos, 1.0f), mvp);
        output.worldNormal = mul(float4(input.normal, 0.f), world).xyz;
        return output; 
    }

    )";

static constexpr auto fs = R"(
    cbuffer ConstantBuffer : register(b0)
    {
        float4x4 world;
        float4x4 mvp;
        float4 objectColor;
        float4 lightColor;
        float3 lightDirection;
        float3 lightPosition;
        float time;
        int lightType;
    };

    struct PSInput
    {
        float4 pos : SV_POSITION;
        float3 worldNormal : NORMAL;
    };

    float4 main(PSInput input) : SV_TARGET
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
            lightVec = lightPosition - input.pos.xyz;
            float distance = length(lightVec);
            attenuation = 1.0f / (1.0f + 0.09f * distance + 0.032f * distance * distance);
        }   
       
        float diffuseFactor = max(dot(normal, lightVec), 0.0);
        float4 diffuseColor = lightColor * diffuseFactor; 

        float4 ambientColor = objectColor * 0.1;

        return float4(((ambientColor + (diffuseColor* attenuation)) * objectColor ).xyz, 1.0);
    }

    )";

}  // namespace Shaders::Basic
