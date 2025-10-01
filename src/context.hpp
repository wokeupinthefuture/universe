#pragma once

#include "common/memory.hpp"
#include "common/heap_array.hpp"
#include "renderer.hpp"
#include "entity.hpp"
#include "input.hpp"

struct GameState
{
    Entity* grid;
    Entity* sphere;
};

struct Context
{
    bool wantsToQuit;
    vec2 windowSize;
    Platform::Window window;

    Arena permanentMemory;
    Arena tempMemory;

    InputState input;
    RenderState render;

    EntityManager entityManager;

    GameState gameState;
};

inline void contextInit(Context& context, size_t memorySize, size_t tempMemorySize)
{
    arenaInit(context.permanentMemory, memorySize);
    arenaInit(context.tempMemory, tempMemorySize);

    arrayAlloc(context.render.drawCommands, context.permanentMemory);
    arrayAlloc(context.entityManager.entities, context.permanentMemory);
}

inline void contextDeinit(Context& context)
{
    arenaDeinit(context.permanentMemory);
    arenaDeinit(context.tempMemory);
}

#define CTX(voidPtr) (*(Context*)(voidPtr))
