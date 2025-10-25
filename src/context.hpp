#pragma once

#include "common/memory.hpp"
#include "platform.hpp"
#include "renderer.hpp"
#include "entity.hpp"
#include "input.hpp"
#include "gui.hpp"

struct CameraController
{
    static constexpr auto PITCH_YAW_SMOOTHING = 30.f;
    static constexpr auto DEFAULT_SPEED = 5.f;
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
    Entity* quad;
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

    Arena platformMemory;  // lives the entire app
    Arena gameMemory;      // lives the entire app, cleared on hot-reload
    Arena tempMemory;      // lives for the duration of a frame

    PlatformToGameBuffer platform;
    InputState input;
    RenderState render;
    EntityManager entityManager;
    GuiState gui;

    GameState gameState;
};

inline Context* g_context;

void contextInit(Context& context, size_t memorySize, size_t tempMemorySize);
void contextHotReload(Context& context);
void contextDeinit(Context& context);
