#pragma once

#include "common.hpp"
#include "../platform.hpp"

struct Arena
{
    u8* buffer;
    size_t size;
    size_t previousOffset;
    size_t currentOffset;
};

inline void arenaInit(Arena& arena, size_t sizeBytes, void* startAddr = nullptr)
{
    arena.buffer = (u8*)Platform::allocMemory(sizeBytes, startAddr);
    arena.size = sizeBytes;
    arena.currentOffset = 0;
    arena.previousOffset = 0;
}

inline void* arenaAlloc(Arena& arena, size_t size, ptrdiff_t aligning)
{
    assert(size != 0);
    assert(!(aligning & (aligning - 1)));

    const auto remainder = size & (aligning - 1);
    const auto padding = remainder != 0 ? (aligning - remainder) : 0;
    const auto sizeWithPadding = size + padding;
    const auto currentOffset = arena.currentOffset;
    const auto ptr = arena.buffer + currentOffset;

    if (arena.currentOffset + sizeWithPadding <= arena.size)
    {
        memset(ptr, 0, sizeWithPadding);
        arena.currentOffset += sizeWithPadding;
        return ptr;
    }
    else
    {
        return nullptr;
    }
}

template <typename T>
inline T* arenaAllocArray(Arena& arena, size_t count)
{
    return (T*)arenaAlloc(arena, sizeof(T) * count, alignof(T));
}

template <typename T>
inline T* arenaAllocTyped(Arena& arena)
{
    return arenaAllocArray<T>(arena, 1);
}

inline void arenaFreeAll(Arena& arena)
{
    arena.currentOffset = 0;
    arena.previousOffset = 0;
}

inline void arenaDeinit(Arena& arena)
{
    if (arena.buffer && arena.size > 0)
        Platform::freeMemory(arena.buffer, arena.size);

    arena.buffer = nullptr;
    arena.size = 0;
    arena.previousOffset = 0;
    arena.currentOffset = 0;
}
