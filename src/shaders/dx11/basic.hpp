#pragma once

namespace Shaders::Basic
{

struct alignas(16) VariablesVS
{
    float mvp[4][4];
    float time;
    float padding[3];
};
static_assert(sizeof(VariablesVS) % 16 == 0, "Constant buffer size must be a multiple of 16 bytes");

static constexpr VariablesVS DEFAULT_VARIABLES_VS = {{}, 0.f, {}};

struct alignas(16) VariablesPS
{
    float color[4];
};
static_assert(sizeof(VariablesPS) % 16 == 0, "Constant buffer size must be a multiple of 16 bytes");

static constexpr VariablesPS DEFAULT_VARIABLES_PS = {{0.5f, 0.f, 0.5f, 1.f}};

static constexpr auto vs = R"(

    cbuffer BasicVSConstantBuffer : register(b0)
    {
        float4x4 mvp;
        float time;
        float3 padding;
    };

    struct VSInput
    {
        float3 pos : POSITION;
    };

    struct VSOutput
    {
        float4 pos : SV_POSITION;
    };


    VSOutput main(VSInput input)
    {
        VSOutput output;
        output.pos = mul(float4(input.pos, 1.0f), mvp);
        return output; 
    }

    )";

static constexpr auto fs = R"(
    cbuffer BasicPSConstantBuffer : register(b1)
    {
        float4 color;
    };

    float4 main() : SV_TARGET
    {
        return color;
    }

    )";

}  // namespace Shaders::Basic
