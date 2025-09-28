#pragma once

namespace Shaders::Basic
{

static constexpr auto vs = R"(

    cbuffer BasicVSConstantBuffer : register(b0)
    {
        float4x4 mvp;
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
