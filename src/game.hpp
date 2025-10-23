#pragma once

#include "context.hpp"

extern "C"
{
    GAME_API void gameInit(Context& ctx);
    GAME_API void gameUpdateAndRender(Context& ctx);
    GAME_API void gamePreHotReload(Context& ctx);
    GAME_API void gamePostHotReload(Context& ctx);
    GAME_API void gameExit(Context& ctx);
}
