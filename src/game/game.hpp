#pragma once

#include "app/platform.hpp"

extern "C"
{
    GAME_API void gameInit(void* contextPtr);
    GAME_API void gameUpdate(void* contextPtr);
    GAME_API void gamePreHotReload(void* contextPtr);
    GAME_API void gamePostHotReload(void* contextPtr);
}