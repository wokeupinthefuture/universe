#pragma once

#include "memory.hpp"
#include "input.hpp"
#include "renderer.hpp"

struct Context
{
    Platform::Window window = nullptr;

    Platform::InputState input;
    Renderer::RenderState render;

    Arena gameMemory;
    Arena tempMemory;
};

inline void contextInit(Context& context, size_t memorySize, size_t tempMemorySize)
{
    Platform::contextInput = &context.input;
    arenaInit(context.gameMemory, memorySize);
    arenaInit(context.tempMemory, tempMemorySize);
}

inline void contextDeinit(Context& context)
{
    Platform::contextInput = nullptr;
    arenaDeinit(context.gameMemory);
    arenaDeinit(context.tempMemory);
}

#define CTX(voidPtr) (*(Context*)(voidPtr))
