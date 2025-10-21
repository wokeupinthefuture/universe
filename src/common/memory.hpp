#pragma once

#include "common.hpp"
#include "../platform.hpp"

struct Arena
{
    u8* buffer;
    size_t size;
    size_t used;
};

inline void arenaInit(Arena& arena, size_t sizeBytes, void* startAddr = nullptr)
{
    arena.buffer = (u8*)Platform::allocMemory(sizeBytes, startAddr);
    arena.size = sizeBytes;
    arena.used = 0;
}

inline void* arenaAlloc(Arena& arena, size_t allocSize, size_t allocAlign = alignof(decltype(*Arena::buffer)))
{
    const auto alignmentIsPowerOfTwo = !(allocAlign & (allocAlign - 1));
    assert(allocSize != 0);
    assert(allocAlign != 0 && alignmentIsPowerOfTwo);

    const auto currentPtr = (uintptr_t)(arena.buffer + arena.used);
    const auto remainder = currentPtr & (allocAlign - 1);
    const auto padding = (allocAlign - remainder) % allocAlign;

    assert(arena.used + allocSize + padding <= arena.size);

    const auto ptr = arena.buffer + arena.used + padding;
    memset(ptr, 0, allocSize);
    arena.used += allocSize + padding;
    return ptr;
}

template <typename T>
inline T* arenaAlloc(Arena& arena, size_t count)
{
    return (T*)arenaAlloc(arena, sizeof(T) * count, alignof(T));
}

template <typename T>
inline T* arenaAlloc(Arena& arena)
{
    return (T*)arenaAlloc(arena, sizeof(T), alignof(T));
}

inline void arenaFreeAll(Arena& arena)
{
    arena.used = 0;
}

inline void arenaDeinit(Arena& arena)
{
    if (arena.buffer && arena.size > 0)
        Platform::freeMemory(arena.buffer, arena.size);

    arena.buffer = nullptr;
    arena.size = 0;
    arena.used = 0;
}
