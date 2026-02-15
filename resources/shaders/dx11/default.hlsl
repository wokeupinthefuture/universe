#include "variables.hlsl"

Texture2D diffuseTexture : register(t0);
SamplerState texSampler : register(s0);

struct VSInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float4 clipPos : SV_POSITION;
    float3 worldPos : WPOSITION;
    float3 worldNormal : NORMAL;
    float2 uv : TEXCOORD;
};

#define BIT(b) (1 << (b))

static const int SHADER_TRAIT_SHADED   = BIT(0);
static const int SHADER_TRAIT_TEXTURED = BIT(1);
static const int SHADER_TRAIT_CIRCLE   = BIT(2);

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
    float4 color;

    if (v_shaderTraits & SHADER_TRAIT_TEXTURED)
    {
        float4 textureSample = diffuseTexture.Sample(texSampler, input.uv);
        color = textureSample * v_objectColor;
        if (all(textureSample == float4(0,0,0,0)))
            color = float4(1, 0, 1, 1);
    }
    else
    {
        color = v_objectColor;
    }

    if (v_shaderTraits & SHADER_TRAIT_CIRCLE)
    {
        float2 center = input.uv - 0.5f;
        if (dot(center, center) > 0.25f)
            discard;
    }

    if (v_shaderTraits & SHADER_TRAIT_SHADED)
    {
        float3 normal = normalize(input.worldNormal);
        float3 lightVec = 0.0f;
        float attenuation = 1.f;
        const float luminosity = 0.1f;

        if (v_lightType == 0)
        {
            lightVec = -v_lightDirection;
        }
        else if (v_lightType == 1)
        {
            lightVec = v_lightPosition - input.worldPos.xyz;
            float distance = length(lightVec) * luminosity;
            attenuation = 1.f / (1.f + 0.09f * distance + 0.032f * distance * distance);
        }

        float diffuseFactor = max(dot(normal, normalize(lightVec)), 0.0);
        float4 diffuseColor = v_lightColor * diffuseFactor;
        float4 ambientColor = color * 0.3;

        color = float4(((ambientColor + (diffuseColor * attenuation)) * color).xyz, color.a);
    }

    return color;
}
