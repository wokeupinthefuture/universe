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
    Mesh* mesh;
    ShaderType shader;
    ShaderVariable variables[MAX_SHADER_VARIABLES];
};

static constexpr auto MAX_DRAW_COMMANDS = 10;
static constexpr auto MAX_LOADED_MESHES = 10;

struct RenderState
{
    bool needsToResize;
    vec2 screenSize;
    HeapArray<DrawCommand> drawCommands;
    Mesh generatedMeshes[(i32)MeshType::Max];
    Mesh loadedMeshes[MAX_LOADED_MESHES];
};

void renderInit(RenderState& state, void* window);
void renderDeinit();

void createShaderVariables(DrawCommand& command);

inline DrawCommand* pushDrawCmd(RenderState& state, MeshType meshType, ShaderType shader = ShaderType::Basic)
{
    DrawCommand cmd = {};
    cmd.mesh = &state.generatedMeshes[(i32)meshType];
    cmd.rasterizerState = RasterizerState::Default;
    cmd.shader = shader;

    if (meshType == MeshType::Grid)
    {
        cmd.rasterizerState = RasterizerState::Wireframe;
        cmd.shader = ShaderType::Unlit;
    }

    createShaderVariables(cmd);

    return arrayPush(state.drawCommands, cmd);
}

void renderClearAndResize(RenderState& state, glm::vec4 color);
void renderDraw(DrawCommand const& command);
void renderPresent();
