#pragma once

#include "common.hpp"
#include "common/log.hpp"

struct Arena
{
    u8* buffer;
    size_t size;
    size_t used;
    size_t prevUsed;
};

// calling platform to allocate and free, only compiled in main exe
void arenaInit(Arena& arena, size_t sizeBytes, void* startAddr = nullptr);
void arenaDeinit(Arena& arena);

inline void* arenaAlloc(Arena& arena, size_t allocSize, size_t allocAlign)
{
    const auto alignmentIsPowerOfTwo = !(allocAlign & (allocAlign - 1));
    assert(allocSize != 0);
    assert(allocAlign != 0 && alignmentIsPowerOfTwo);

    const auto currentPtr = (uintptr_t)(arena.buffer + arena.used);
    const auto remainder = currentPtr & (allocAlign - 1);
    const auto padding = remainder ? (allocAlign - remainder) : 0;
    assert(arena.used + allocSize + padding <= arena.size);

    const auto ptr = arena.buffer + arena.used + padding;

    logInfo("arena allocated %.3f mb / %.2f kb / %llu bytes (padding: %llu), from 0x%llx to 0x%llx",
        std::round((float)allocSize / 1024 / 1024),
        std::round((float)allocSize / 1024),
        allocSize + padding,
        padding,
        ptr,
        ptr + allocSize);

    memset(ptr, 0, allocSize);
    arena.prevUsed = arena.used;
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

inline void arenaClear(Arena& arena)
{
    arena.used = 0;
    arena.prevUsed = 0;
}
