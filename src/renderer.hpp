#pragma once

#include "geometry.hpp"
#include "common/heap_array.hpp"

struct DrawCommand;

void setShaderVariableInt(DrawCommand& command, const char* variableName, int value);
void setShaderVariableFloat(DrawCommand& command, const char* variableName, float value);
void setShaderVariableVec2(DrawCommand& command, const char* variableName, vec2 value);
void setShaderVariableVec3(DrawCommand& command, const char* variableName, vec3 value);
void setShaderVariableVec4(DrawCommand& command, const char* variableName, vec4 value);
void setShaderVariableMat4(DrawCommand& command, const char* variableName, mat4 value);

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

enum class RasterizerState
{
    Default,
    Wireframe,
    Max
};

struct DrawCommand
{
    RasterizerState rasterizerState;
    MeshType mesh;
    ShaderType shader;
    ShaderVariable variables[MAX_SHADER_VARIABLES];
};

static constexpr auto MAX_DRAW_COMMANDS = 10;

struct RenderState
{
    HeapArray<DrawCommand> drawCommands;
};

void renderInit(void* window, float windowWidth, float windowHeight);
void renderDeinit();

void createShaderVariables(DrawCommand& command);

inline DrawCommand* pushDrawCmd(HeapArray<DrawCommand>& drawCommands, MeshType mesh, ShaderType shader = ShaderType::Basic)
{
    DrawCommand cmd = {};
    cmd.mesh = mesh;
    cmd.rasterizerState = RasterizerState::Default;
    cmd.shader = shader;

    if (mesh == MeshType::Grid)
    {
        cmd.rasterizerState = RasterizerState::Wireframe;
        cmd.shader = ShaderType::Unlit;
    }

    createShaderVariables(cmd);

    return arrayPush(drawCommands, cmd);
}

void renderDraw(DrawCommand const& command);
void renderClear(glm::vec4 color);
void renderPresent();
