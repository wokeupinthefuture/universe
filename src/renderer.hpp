#pragma once

#include "geometry.hpp"
#include "common/heap_array.hpp"

#include "shaders.hpp"

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
    bool needsToResize;
    vec2 screenSize;
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

void renderClearAndResize(RenderState& state, glm::vec4 color);
void renderDraw(DrawCommand const& command);
void renderPresent();
