#pragma once

#include "common/common.hpp"

struct DrawCommand;

void setShaderVariableInt(DrawCommand& command, const char* variableName, int value);
void setShaderVariableFloat(DrawCommand& command, const char* variableName, float value);
void setShaderVariableVec2(DrawCommand& command, const char* variableName, vec2 value);
void setShaderVariableVec3(DrawCommand& command, const char* variableName, vec3 value);
void setShaderVariableVec4(DrawCommand& command, const char* variableName, vec4 value);
void setShaderVariableMat4(DrawCommand& command, const char* variableName, mat4 value);
int getShaderVariableInt(DrawCommand& command, const char* variableName);
float getShaderVariableFloat(DrawCommand& command, const char* variableName);
vec2 getShaderVariableVec2(DrawCommand& command, const char* variableName);
vec3 getShaderVariableVec3(DrawCommand& command, const char* variableName);
vec4 getShaderVariableVec4(DrawCommand& command, const char* variableName);
mat4 getShaderVariableMat4(DrawCommand& command, const char* variableName);

enum class ShaderType
{
    Basic,
    Unlit,
    Max
};

static constexpr auto MAX_SHADER_VARIABLES = 10;

union ShaderVariableValue
{
    int i;
    float f;
    vec2 v2;
    vec3 v3;
    vec4 v4;
    mat4 m4;
};

struct ShaderVariable
{
    const char* name;
    ShaderVariableValue value;
};

namespace Shaders
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
    .objectColor = {1.f, 1.f, 1.f, 1.f},
    .lightColor = {1.f, 1.f, 1.f, 1.f},
    .lightDirection = {0.f, 0.f, 1.f},
    ._padding0 = {},
    .lightPosition = {},
    .time = 0.f,
    .lightType = 0,
    ._padding1 = {},
};

namespace Basic
{
static constexpr auto PATH = L"resources/shaders/dx11/basic.hlsl";
}

namespace Unlit
{
static constexpr auto PATH = L"resources/shaders/dx11/unlit.hlsl";
}

}  // namespace Shaders
