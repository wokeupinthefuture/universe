#pragma once

#include <type_traits>
#include "raylib.h"

template <typename T>
void setShaderUniform(Shader shader, int loc, T value)
{
    if constexpr (std::is_same_v<T, float>)
    {
        SetShaderValue(shader, loc, &value, SHADER_UNIFORM_FLOAT);
    }
    else if constexpr (std::is_same_v<T, int>)
    {
        SetShaderValue(shader, loc, &value, SHADER_UNIFORM_INT);
    }
    else if constexpr (std::is_same_v<T, Vector2>)
    {
        SetShaderValue(shader, loc, &value.x, SHADER_UNIFORM_VEC2);
    }
    else if constexpr (std::is_same_v<T, Vector3>)
    {
        SetShaderValue(shader, loc, &value.x, SHADER_UNIFORM_VEC3);
    }
    else if constexpr (std::is_same_v<T, Vector4> || std::is_same_v<T, Quaternion>)
    {
        SetShaderValue(shader, loc, &value.x, SHADER_UNIFORM_VEC4);
    }
    else if constexpr (std::is_same_v<T, Color>)
    {
        float colorNormalized[4] = {value.r / 255.0f, value.g / 255.0f, value.b / 255.0f, value.a / 255.0f};
        SetShaderValue(shader, loc, colorNormalized, SHADER_UNIFORM_VEC4);
    }
    else if constexpr (std::is_same_v<T, Matrix>)
    {
        SetShaderValueMatrix(shader, loc, value);
    }
    else
    {
        static_assert(!sizeof(T), "Unsupported uniform type");
    }
}
