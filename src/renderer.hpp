#pragma once

#include "geometry.hpp"
#include "texture.hpp"
#include "common/heap_array.hpp"

#include "shaders.hpp"

enum class RasterizerState
{
    Default,
    Wireframe,
    Max
};

enum class DrawFlag
{
    Active = BIT(0),
    DepthWrite = BIT(1)
};

DEFINE_ENUM_BITWISE_OPERATORS(DrawFlag);

static constexpr auto MAX_TEXTURE_SLOTS = 5;
struct DrawCommand
{
    DrawFlag flags;
    RasterizerState rasterizerState;
    Mesh* mesh;
    ShaderType shader;
    ShaderVariable variables[MAX_SHADER_VARIABLES];
    Texture* textures[MAX_TEXTURE_SLOTS];
};

struct RenderState
{
    bool needsToResize;
    vec2 screenSize;
    HeapArray<DrawCommand> drawCommands;
    Mesh generatedMeshes[(i32)GeneratedMesh::Max];
    HeapArray<Mesh> loadedMeshes;
    HeapArray<Texture> loadedTextures;
};

inline Mesh* findMeshByName(RenderState& state, String name)
{
    return find(state.loadedMeshes.data, state.loadedMeshes.size, [name](Mesh& mesh) { return mesh.name == name; });
}

inline Texture* findTextureByName(RenderState& state, String name)
{
    return find(state.loadedTextures.data, state.loadedTextures.size, [name](Texture& texture) { return texture.name == name; });
}

void renderInitResources(RenderState& state, HeapArray<Asset> const& assets);
void renderInit(RenderState& state, void* window);
void renderDeinit();

void createShaderVariables(DrawCommand& command);

inline DrawCommand* pushDrawCmd(RenderState& state, Mesh* mesh, ShaderType shader = ShaderType::Basic)
{
    DrawCommand cmd = {};
    cmd.flags = DrawFlag::Active | DrawFlag::DepthWrite;
    cmd.rasterizerState = RasterizerState::Default;
    cmd.shader = shader;
    cmd.mesh = mesh;
    createShaderVariables(cmd);
    return arrayPush(state.drawCommands, cmd);
}

inline void setTexture(DrawCommand& cmd, size_t textureSlot, Texture& texture)
{
    ENSURE(textureSlot < MAX_TEXTURE_SLOTS);
    cmd.textures[textureSlot] = &texture;
}

void renderClearAndResize(RenderState& state, glm::vec4 color);
void renderDraw(DrawCommand const& command);
void renderPresent();
