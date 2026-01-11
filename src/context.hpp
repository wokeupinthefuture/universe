#pragma once

#include "common/memory.hpp"
#include "platform.hpp"
#include "renderer.hpp"
#include "entity.hpp"
#include "input.hpp"
#include "gui.hpp"

struct Context
{
    bool isGameLoaded;
    bool wantsToQuit;
    bool wantsToReload;

    Arena platformMemory;  // lives the entire app
    Arena gameMemory;      // lives the entire app, cleared on hot-reload
    Arena tempMemory;      // lives for the duration of a frame

    PlatformToGameBuffer platform;
    InputState input;
    RenderState render;
    EntityManager entityManager;
    GuiState gui;

    void* gameState;
};

inline Context* g_context;

void contextInit(Context& context, size_t memorySize, size_t tempMemorySize);
void contextHotReload(Context& context);
void contextDeinit(Context& context);
