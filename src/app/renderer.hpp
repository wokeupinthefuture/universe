#pragma once

#include "lib/common.hpp"
#include "platform.hpp"
#include "lib/heap_array.hpp"
#include "lib/log.hpp"

namespace Renderer
{

enum class ShaderType
{
    Basic,
    Max
};

struct Vertex
{
    vec3 pos;
};

static constexpr Vertex TRIANGLE_VERTICES[] = {
    {.pos = vec3{-0.5f, -0.5f, 0.f}}, {.pos = vec3{0.f, 0.5f, 0.f}}, {.pos = vec3{0.5f, -0.5f, 0.f}}};
static constexpr Vertex QUAD_VERTICES[] = {{.pos = vec3{-0.5f, -0.5f, 0.f}},
    {.pos = vec3{-0.5f, 0.5f, 0.f}},
    {.pos = vec3{0.5f, 0.5f, 0.f}},
    {.pos = vec3{0.5f, -0.5f, 0.f}}};

enum class MeshType
{
    Triangle,
    Quad,
    Max
};

struct DrawCommand
{
    MeshType mesh;
    ShaderType shader;
};

struct RenderState
{
    HeapArray<DrawCommand> drawCommands;
};

void init(Platform::Window window, float windowWidth, float windowHeight);
void deinit();

inline size_t addDrawCommand(HeapArray<DrawCommand>& commands, DrawCommand command)
{
    const auto idx = commands.size;
    arrayPush(commands, command);
    return idx;
}

inline size_t addDrawTriangle(HeapArray<DrawCommand>& commands)
{
    return addDrawCommand(commands, {.mesh = Renderer::MeshType::Triangle, .shader = Renderer::ShaderType::Basic});
}

void draw(DrawCommand const& command);
void clear(glm::vec4 color);
void present();

void setShaderVariableInt(ShaderType shader, const char* variableName, int value);
void setShaderVariableFloat(ShaderType shader, const char* variableName, float value);
void setShaderVariableVec2(ShaderType shader, const char* variableName, vec2 value);
void setShaderVariableVec3(ShaderType shader, const char* variableName, vec3 value);
void setShaderVariableVec4(ShaderType shader, const char* variableName, vec4 value);
void setShaderVariableMat4(ShaderType shader, const char* variableName, mat4 value);

mat4 getShaderVariableMat4(ShaderType shader, const char* variableName);

}  // namespace Renderer
