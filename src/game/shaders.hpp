#pragma once

#if 0
#include "raylib.h"
#include "rlgl.h"
#include "lib/heap_array.hpp"
#include "app/memory.hpp"
#include "lib/log.hpp"

#define LOAD_SHADER_FROM_SOURCE true
#include "shaders/light.hpp"
#include "shaders/basic.hpp"

#include "imgui.h"

#include "math.hpp"

struct ShaderUniform
{
    char name[128];
    int location;
    ShaderUniformDataType type;
};

using ShaderUniforms = HeapArray<ShaderUniform>;

struct ShaderEx
{
    Shader base;
    char name[128];
    ShaderUniforms uniforms;
};

inline void fillShaderUniforms(ShaderEx& shader, Arena& arena)
{
    int uniformsCount = 0;
    rlGetProgram(shader.base.id, RL_ACTIVE_UNIFORMS, &uniformsCount);

    int maxNameLength = 0;
    rlGetProgram(shader.base.id, RL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);

    char* uniformName = arenaAllocArray<char>(arena, maxNameLength);

    for (int i = 0; i < uniformsCount; i++)
    {
        i32 length;
        i32 size;
        i32 rawType;
        rlGetActiveUniform(shader.base.id, i, maxNameLength, &length, &size, &rawType, uniformName);

        ShaderUniformDataType type{};

        switch (rawType)
        {
            case RL_FLOAT: type = ShaderUniformDataType::SHADER_UNIFORM_FLOAT; break;
            case RL_FLOAT_VEC2: type = ShaderUniformDataType::SHADER_UNIFORM_VEC2; break;
            case RL_FLOAT_VEC3: type = ShaderUniformDataType::SHADER_UNIFORM_VEC3; break;
            case RL_FLOAT_VEC4: type = ShaderUniformDataType::SHADER_UNIFORM_VEC4; break;
            case RL_UNSIGNED_INT: type = ShaderUniformDataType::SHADER_UNIFORM_UINT; break;
            case RL_INT: type = ShaderUniformDataType::SHADER_UNIFORM_INT; break;
            case RL_BOOL: type = ShaderUniformDataType::SHADER_UNIFORM_INT; break;
            default: logError("unsupported uniform type"); break;
        }

        logInfo("Uniform #%d: Name: %s, Type: %x", i, uniformName, rawType);
        arrayPush(shader.uniforms, ShaderUniform{.name = {}, .location = i, .type = type});
        strcpy_s(arrayLast(shader.uniforms)->name, uniformName);
    }
}

inline int findUniformLocation(ShaderUniforms& cache, const char* name)
{
    for (const auto& u : cache)
        if (!strcmp(u.name, name))
            return u.location;
    return -1;
}

inline ShaderEx initShader(const char* name, const char* vs, const char* fs, Arena& arena)
{
    ShaderEx shader{};

    strcpy_s(shader.name, name);

#if LOAD_SHADER_FROM_SOURCE == false
    shader.base = LoadShader(vs, fs);
#else
    shader.base = LoadShaderFromMemory(vs, fs);
#endif

    shader.base.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader.base, "viewPos");

    fillShaderUniforms(shader, arena);

    return shader;
}

inline void deinitShader(ShaderEx& shader)
{
    UnloadShader(shader.base);
    arrayClear(shader.uniforms);
}

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
    else if constexpr (std::is_same_v<T, Vector2> || std::is_same_v<T, vec2>)
    {
        SetShaderValue(shader, loc, &value.x, SHADER_UNIFORM_VEC2);
    }
    else if constexpr (std::is_same_v<T, Vector3> || std::is_same_v<T, vec3>)
    {
        SetShaderValue(shader, loc, &value.x, SHADER_UNIFORM_VEC3);
    }
    else if constexpr (std::is_same_v<T, Vector4> || std::is_same_v<T, Quaternion> || std::is_same_v<T, vec4>)
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

template <typename T>
T getShaderUniform(Shader shader, int loc)
{
    rlEnableShader(shader.id);
    T value{};

    if constexpr (std::is_same_v<T, float>)
    {
        rlGetUniformf(shader.id, loc, &value);
    }
    else if constexpr (std::is_same_v<T, int>)
    {
        rlGetUniformi(shader.id, loc, &value);
    }
    else if constexpr (std::is_same_v<T, Vector2> || std::is_same_v<T, vec2>)
    {
        rlGetUniformf(shader.id, loc, &value.x);
    }
    else if constexpr (std::is_same_v<T, Vector3> || std::is_same_v<T, vec3>)
    {
        rlGetUniformf(shader.id, loc, &value.x);
    }
    else if constexpr (std::is_same_v<T, Vector4> || std::is_same_v<T, Quaternion> || std::is_same_v<T, vec4>)
    {
        rlGetUniformf(shader.id, loc, &value.x);
    }
    else if constexpr (std::is_same_v<T, Color>)
    {
        float color[4];
        rlGetUniformf(shader.id, loc, color);
        value.r = (unsigned char)(color[0] * 255.0f);
        value.g = (unsigned char)(color[1] * 255.0f);
        value.b = (unsigned char)(color[2] * 255.0f);
        value.a = (unsigned char)(color[3] * 255.0f);
    }
    else
    {
        static_assert(!sizeof(T), "Unsupported uniform type");
    }

    return value;
}

inline void debugDrawUniform(ShaderEx shader, ShaderUniform uniform)
{
    if (uniform.type == SHADER_UNIFORM_FLOAT)
    {
        ImGui::Text("%s", uniform.name);
        ImGui::SameLine();
        auto currentValue = getShaderUniform<float>(shader.base, uniform.location);
        // if (ImGui::DragFloat(uniform.name, &currentValue, 0.01f))
        // {
        //     SetShaderValue(shader.base, uniform.location, &currentValue, SHADER_UNIFORM_FLOAT);
        // }
    }

    return;
    if (uniform.type == SHADER_UNIFORM_INT)
    {
        ImGui::Text("%s", uniform.name);
        ImGui::SameLine();
        auto currentValue = getShaderUniform<int>(shader.base, uniform.location);
        if (ImGui::DragInt(uniform.name, &currentValue, 1))
        {
            SetShaderValue(shader.base, uniform.location, &currentValue, SHADER_UNIFORM_INT);
        }
    }
    else if (uniform.type == SHADER_UNIFORM_VEC2)
    {
        ImGui::Text("%s", uniform.name);
        ImGui::SameLine();
        auto currentValue = getShaderUniform<vec2>(shader.base, uniform.location);
        if (ImGui::DragFloat2(uniform.name, &currentValue.x, 0.01f))
        {
            SetShaderValue(shader.base, uniform.location, &currentValue.x, SHADER_UNIFORM_VEC2);
        }
    }
    else if (uniform.type == SHADER_UNIFORM_VEC3)
    {
        ImGui::Text("%s", uniform.name);
        ImGui::SameLine();
        auto currentValue = getShaderUniform<vec3>(shader.base, uniform.location);
        if (ImGui::DragFloat3(uniform.name, &currentValue.x, 0.01f))
        {
            SetShaderValue(shader.base, uniform.location, &currentValue.x, SHADER_UNIFORM_VEC3);
        }
    }
    else if (uniform.type == SHADER_UNIFORM_VEC4)
    {
        ImGui::Text("%s", uniform.name);
        ImGui::SameLine();
        auto currentValue = getShaderUniform<vec4>(shader.base, uniform.location);

        float colorNormalized[4] = {
            currentValue.r / 255.0f, currentValue.g / 255.0f, currentValue.b / 255.0f, currentValue.a / 255.0f};

        if (ImGui::ColorEdit4(uniform.name, colorNormalized))
        {
            currentValue.r = (unsigned char)(colorNormalized[0] * 255.0f);
            currentValue.g = (unsigned char)(colorNormalized[1] * 255.0f);
            currentValue.b = (unsigned char)(colorNormalized[2] * 255.0f);
            currentValue.a = (unsigned char)(colorNormalized[3] * 255.0f);
            SetShaderValue(shader.base, uniform.location, &currentValue.r, SHADER_UNIFORM_VEC4);
        }
    }
    else
    {
        ImGui::Text("Unsupported uniform type: %s:%i", uniform.name, uniform.type);
    }
}

inline void debugDrawShader(ShaderEx shader, const char* ownerName)
{
    char name[128]{};
    sprintf(name, "%s##%s", shader.name, ownerName);
    if (ImGui::TreeNode(name))
    {
        for (const auto& uniform : shader.uniforms)
            debugDrawUniform(shader, uniform);
        ImGui::TreePop();
    }
    ImGui::Separator();
}
#endif