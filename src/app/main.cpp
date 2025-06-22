#include <raylib.h>
#include "lib/common.hpp"
#include "lib/log.hpp"

#include "rlImGui.h"
#include "imgui.h"

#include "context.hpp"
#include "platform.hpp"
#include "game/game.hpp"

#include <thread>

using namespace std::chrono_literals;

struct AppState
{
    bool isRunning = true;
};

static constexpr const char GAME_DLL_NAME[] = "game_" PROJECT_NAME ".dll";
static constexpr const char GAME_DLL_NAME_TEMP[] = "game_" PROJECT_NAME "_temp.dll";

char gameCodeRealPath[256];
char gameCodeTempPath[256];

struct GameCode
{
    void* dll;
    bool isLoaded = false;
    u64 lastWrittenTime{};

    decltype(&gameInit) init;
    decltype(&gameUpdate) update;
    decltype(&gamePreHotReload) preHotReload;
    decltype(&gamePostHotReload) postHotReload;
};

GameCode loadGameCode()
{
    GameCode game{};

    Platform::copyFile(gameCodeRealPath, gameCodeTempPath);
    game.dll = Platform::loadDll(GAME_DLL_NAME_TEMP);
    game.init = Platform_importDllFunc(game.dll, gameInit);
    game.update = Platform_importDllFunc(game.dll, gameUpdate);
    game.preHotReload = Platform_importDllFunc(game.dll, gamePreHotReload);
    game.postHotReload = Platform_importDllFunc(game.dll, gamePostHotReload);
    game.isLoaded = true;
    game.lastWrittenTime = Platform::getFileLastWrittenTime(gameCodeRealPath);

    return game;
}

void unloadGameCode(GameCode& game)
{
    if (game.dll != nullptr)
    {
        Platform::unloadDll(game.dll);
        game.dll = nullptr;
    }
    game.update = nullptr;
    game.preHotReload = nullptr;
    game.postHotReload = nullptr;
    game.init = nullptr;
    game.isLoaded = false;
}

int main()
{
    AppState appState{};

    Context context{};
    contextInit(context, Megabytes(4), Megabytes(4));
    defer({ contextDeinit(context); });

    SetConfigFlags(ConfigFlags::FLAG_VSYNC_HINT);
    InitWindow(1280, 720, PROJECT_NAME);
    DisableCursor();

    rlImGuiSetup(true);

    char directory[256]{};
    Platform::getExeDirectory(directory);
    sprintf(gameCodeRealPath, "%s%s", directory, GAME_DLL_NAME);
    sprintf(gameCodeTempPath, "%s%s", directory, GAME_DLL_NAME_TEMP);

    auto game = loadGameCode();

    game.init((void*)&context);

    while (!WindowShouldClose() && appState.isRunning)
    {
        if (IsKeyPressed(KEY_ESCAPE))
        {
            appState.isRunning = false;
        }

        game.update((void*)&context);

        const auto gameLastWrittenTime = Platform::getFileLastWrittenTime(gameCodeRealPath);
        if (gameLastWrittenTime != game.lastWrittenTime)
        {
            game.preHotReload((void*)&context);
            unloadGameCode(game);
            arenaFreeAll(context.memory);
            std::this_thread::sleep_for(1ms);
            game = loadGameCode();
            game.postHotReload((void*)&context);
        }

        arenaFreeAll(context.tempMemory);
    }

    rlImGuiShutdown();
    CloseWindow();
}