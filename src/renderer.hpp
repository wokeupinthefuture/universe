#pragma once

#include "geometry.hpp"
#include "texture.hpp"
#include "common/array.hpp"

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

    Array<DrawCommand> drawCommands;

    Mesh generatedMeshes[(i32)GeneratedMesh::Max];
    Array<Mesh> meshes;

    Array<Texture> allTextures;
    Array<Texture> spMeshTextures;
    Array<Texture> spCubemaps;
};

inline Mesh& renderGetMesh(RenderState& state, String name)
{
    const auto result = find(state.meshes.data, state.meshes.size, [name](Mesh& mesh) { return mesh.name == name; });
    if (!result)
    {
        logError("%.*s mesh not found", name.length, name.data);
        ENSURE(result);
    }
    return *result;
}

inline Texture& renderGetTexture(RenderState& state, String name)
{
    const auto result =
        find(state.spMeshTextures.data, state.spMeshTextures.size, [name](Texture& texture) { return texture.name == name; });
    if (!result)
    {
        logError("%.*s texture not found", name.length, name.data);
        ENSURE(result);
    }
    return *result;
}

inline Texture& renderGetCubemap(RenderState& state, String name)
{
    const auto result =
        find(state.spCubemaps.data, state.spCubemaps.size, [name](Texture& texture) { return texture.name == name; });
    if (!result)
    {
        logError("%.*s cubemap texture not found", name.length, name.data);
        ENSURE(result);
    }
    return *result;
}

void renderInitResources(RenderState& state, Array<Asset> const& assets);
void renderInit(RenderState& state, void* window);
void renderDeinit();

void createShaderVariables(DrawCommand& command);

inline DrawCommand* pushDrawCmd(RenderState& state, Mesh& mesh, ShaderType shader = ShaderType::Basic)
{
    DrawCommand cmd = {};
    cmd.flags = DrawFlag::Active | DrawFlag::DepthWrite;
    cmd.rasterizerState = RasterizerState::Default;
    cmd.shader = shader;
    cmd.mesh = &mesh;
    createShaderVariables(cmd);
    return arrayPush(state.drawCommands, cmd);
}

void renderClearAndResize(RenderState& state, glm::vec4 color);
void renderDraw(DrawCommand const& command);
void renderPresent();
