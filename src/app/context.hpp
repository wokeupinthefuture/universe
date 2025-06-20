#pragma once

#include "memory.hpp"
#define NO_FONT_AWESOME
#include "imgui.h"
#include "third_party/rlImGui/rlImGui.h"

struct RlImGuiSharedContext
{
    ImGuiContext* context;
    void* rlImGuiBegin;
    void* rlImGuiEnd;
};

struct Context
{
    Arena memory;
    Arena tempMemory;
    RlImGuiSharedContext rlImgui;
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