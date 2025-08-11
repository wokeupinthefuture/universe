#pragma once

namespace Shaders::Basic
{

static constexpr auto vs = R"(

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
        output.pos = float4(input.pos, 1.0f);
        return output; 
    }

    )";

static constexpr auto fs = R"(

    float4 main() : SV_TARGET
    {
        return float4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    )";

}  // namespace Shaders::Basic
