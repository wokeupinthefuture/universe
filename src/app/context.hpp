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

inline void contextInit(Context& context)
{
    arenaInit(context.memory, Megabytes(64));
    arenaInit(context.tempMemory, Megabytes(2));
}

inline void contextDeinit(Context& context)
{
    arenaDeinit(context.memory);
    arenaDeinit(context.tempMemory);
}

#define CTX(voidPtr) (*(Context*)(voidPtr))