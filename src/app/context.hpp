#pragma once

#include "memory.hpp"

struct Context
{
    Arena memory;
    Arena tempMemory;
};

inline void contextInit(Context& context, size_t memorySize, size_t tempMemorySize)
{
    arenaInit(context.memory, memorySize);
    arenaInit(context.tempMemory, tempMemorySize);
}

inline void contextDeinit(Context& context)
{
    arenaDeinit(context.memory);
    arenaDeinit(context.tempMemory);
}

#define CTX(voidPtr) (*(Context*)(voidPtr))