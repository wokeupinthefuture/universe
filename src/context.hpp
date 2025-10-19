#pragma once

#include "common/memory.hpp"
#include "common/heap_array.hpp"
#include "platform.hpp"
#include "renderer.hpp"
#include "entity.hpp"
#include "input.hpp"
#include "gui.hpp"

struct CameraController
{
    static constexpr auto PITCH_YAW_SMOOTHING = 30.f;
    static constexpr auto DEFAULT_SPEED = 10.f;
    Entity* camera;
    bool isPressed;
    vec2 pressedPos;
    float speed;
    float sensitivity;
    vec3 pitchYaw;
    vec3 pitchYawTarget;
    vec3 pitchYawDelta;
};

struct GameState
{
    Entity* grid;
    Entity* sphere;
    Entity* cube;
    CameraController cameraController;
    Entity* light;
    Entity* lightOrigin;
    Entity* arrow;
};

struct Context
{
    bool pause;
    float timeScale;
    float dt;

    bool wantsToQuit;
    bool wantsToReload;

    Arena platformMemory;
    Arena gameMemory;
    Arena tempMemory;

    PlatformToGameBuffer platform;
    InputState input;
    RenderState render;
    EntityManager entityManager;
    GameState gameState;
    GuiState gui;
};

inline void contextInit(Context& context, size_t memorySize, size_t tempMemorySize)
{
    context.timeScale = 1.f;
    context.dt = 0.016f;
    context.render.needsToResize = true;

    arenaInit(context.platformMemory, memorySize);
    arenaInit(context.gameMemory, memorySize);
    arenaInit(context.tempMemory, tempMemorySize);

    arrayAlloc(context.render.drawCommands, context.gameMemory);
    arrayAlloc(context.entityManager.entities, context.gameMemory);
}

inline void contextClear(Context& context)
{
    arrayClear(context.entityManager.entities);
    arrayClear(context.render.drawCommands);

    arenaFreeAll(context.gameMemory);
    arenaFreeAll(context.tempMemory);
}

inline void contextDeinit(Context& context)
{
    arenaDeinit(context.gameMemory);
    arenaDeinit(context.tempMemory);
    arenaDeinit(context.platformMemory);
}
