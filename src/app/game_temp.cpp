#include "app/context.hpp"
#include "app/entity.hpp"
#include "app/input.hpp"
#include "app/renderer.hpp"
#include "lib/log.hpp"

struct GameState
{
    EDrawable* triangle;
};

void gameInit(void* contextPtr)
{
    auto& context = CTX(contextPtr);
    GameState& game = *(GameState*)context.gameMemory.buffer;

    const auto triangleCommand = context.render.drawCommands[Renderer::addDrawTriangle(context.render.drawCommands)];
    game.triangle = &drawables[0];
    game.triangle->shader = triangleCommand.shader;
    realDrawables++;

    Renderer::setShaderVariableVec4(game.triangle->shader, "color", vec4(0.5, 1, 0.5, 1));
}

void gameUpdate(void* contextPtr)
{
    using namespace Platform;

    auto& context = CTX(contextPtr);
    GameState& game = *(GameState*)context.gameMemory.buffer;

    if (isKeyPressed(KeyboardKey::KEY_D))
    {
        addLocalPosition(*game.triangle, vec3(0.1f, 0, 0));
    }
    if (isKeyPressed(KeyboardKey::KEY_A))
    {
        addLocalPosition(*game.triangle, vec3(-0.1f, 0, 0));
    }
    if (isKeyPressed(KeyboardKey::KEY_W))
    {
        addLocalPosition(*game.triangle, vec3(0.f, 0, 0.1f));
    }
    if (isKeyPressed(KeyboardKey::KEY_S))
    {
        addLocalPosition(*game.triangle, vec3(0, 0, -0.1f));
    }
}
