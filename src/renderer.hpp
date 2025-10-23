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

struct RenderState
{
    bool needsToResize;
    vec2 screenSize;
    HeapArray<DrawCommand> drawCommands;
    Mesh generatedMeshes[(i32)GeneratedMesh::Max];
    HeapArray<Mesh> loadedMeshes;
};

inline Mesh* findMeshByName(HeapArray<Mesh> const& meshes, String name)
{
    return find(meshes.data, meshes.size, [name](Mesh& mesh) { return mesh.name == name; });
}

void renderInitGeometry(RenderState& state, HeapArray<Asset> const& assets);
void renderInit(RenderState& state, void* window);
void renderDeinit();

void createShaderVariables(DrawCommand& command);

inline DrawCommand* pushDrawCmd(RenderState& state, Mesh* mesh, ShaderType shader = ShaderType::Basic)
{
    DrawCommand cmd = {};
    cmd.rasterizerState = RasterizerState::Default;
    cmd.shader = shader;
    cmd.mesh = mesh;
    createShaderVariables(cmd);
    return arrayPush(state.drawCommands, cmd);
}

void renderClearAndResize(RenderState& state, glm::vec4 color);
void renderDraw(DrawCommand const& command);
void renderPresent();
