#include "memory.hpp"
#include "../platform.hpp"

void arenaInit(Arena& arena, size_t sizeBytes, void* startAddr)
{
    arena.buffer = (u8*)Platform::allocMemory(sizeBytes, startAddr);
    arena.size = sizeBytes;
    arena.used = 0;
    arena.prevUsed = 0;
}

void arenaDeinit(Arena& arena)
{
    if (arena.buffer && arena.size > 0)
        Platform::freeMemory(arena.buffer, arena.size);

    arena.buffer = nullptr;
    arena.size = 0;
    arena.used = 0;
    arena.prevUsed = 0;
}
