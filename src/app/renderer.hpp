#pragma once

#include "lib/common.hpp"
#include "platform.hpp"
#include "lib/heap_array.hpp"

namespace Renderer
{

enum ShaderType
{
    Basic
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

struct Mesh
{
    enum Type
    {
        Triangle,
        Quad,
        Max
    };

    Type type;
};

struct DrawCommand
{
    Mesh mesh;
    ShaderType shader;
};

inline HeapArray<DrawCommand> drawCommands;

void init(Platform::Window window, float windowWidth, float windowHeight);
void deinit();

void addDrawCommand(DrawCommand command);

void draw(DrawCommand const& command);
void clear(glm::vec4 color);
void present();

}  // namespace Renderer
