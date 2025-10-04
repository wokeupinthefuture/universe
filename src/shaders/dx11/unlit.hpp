#pragma once

namespace Shaders::Unlit
{

struct Variables
{
    float mvp[4][4];
    float objectColor[4];
    float time;
    float pad[3];
};
static_assert(sizeof(Variables) % 16 == 0, "Constant buffer size must be a multiple of 16 bytes");

static constexpr Variables DEFAULT_VARIABLES = {.mvp = {}, .objectColor = {0.5f, 0.f, 0.5f, 1.f}, .time = {}, .pad = {}};

static constexpr auto vs = R"(
    cbuffer ConstantBuffer : register(b0)
    {
        float4x4 mvp;
        float4 objectColor;
        float time;
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
        return output; 
    }

    )";

static constexpr auto fs = R"(
    cbuffer ConstantBuffer : register(b0)
    {
        float4x4 mvp;
        float4 objectColor;
        float time;
    };

    struct PSInput
    {
        float4 pos : SV_POSITION;
        float3 worldNormal : NORMAL;
    };

    float4 main(PSInput input) : SV_TARGET
    {   
        return float4(objectColor.x, objectColor.y, objectColor.z, 1.0);
    }

    )";

}  // namespace Shaders::Unlit
