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
};

struct Context
{
    bool pause;
    float timeScale;
    float dt;

    bool wantsToQuit;
    bool wantsToReload;

    Arena permanentMemory;
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

    arenaInit(context.permanentMemory, memorySize);
    arenaInit(context.tempMemory, tempMemorySize);

    arrayAlloc(context.render.drawCommands, context.permanentMemory);
    arrayAlloc(context.entityManager.entities, context.permanentMemory);
}

inline void contextClear(Context& context)
{
    arrayClear(context.entityManager.entities);
    arrayClear(context.render.drawCommands);

    arenaFreeAll(context.permanentMemory);
    arenaFreeAll(context.tempMemory);
}

inline void contextDeinit(Context& context)
{
    arenaDeinit(context.permanentMemory);
    arenaDeinit(context.tempMemory);
}

#define CTX(voidPtr) (*(Context*)(voidPtr))
