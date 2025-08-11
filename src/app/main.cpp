#include "input.hpp"
#include "lib/common.hpp"
#include "lib/log.hpp"

#include "imgui.h"

#include "context.hpp"
#include "platform.hpp"
#include "game/game.hpp"
#include "input.hpp"
#include "renderer.hpp"

#include <thread>
#include <chrono>

using namespace std::chrono_literals;

struct AppState
{
    bool isRunning = true;
    Platform::Window window = nullptr;
};

static constexpr const char GAME_DLL_NAME[] = "game_" PROJECT_NAME ".dll";
static constexpr const char GAME_DLL_NAME_TEMP[] = "game_" PROJECT_NAME "_temp.dll";

char gameCodeRealPath[256];
char gameCodeTempPath[256];

struct GameCode
{
    void* lib;
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
    game.lib = Platform::loadDynamicLib(GAME_DLL_NAME_TEMP);
    game.init = Platform_loadDynamicFunc(game.lib, gameInit);
    game.update = Platform_loadDynamicFunc(game.lib, gameUpdate);
    game.preHotReload = Platform_loadDynamicFunc(game.lib, gamePreHotReload);
    game.postHotReload = Platform_loadDynamicFunc(game.lib, gamePostHotReload);
    game.isLoaded = true;
    game.lastWrittenTime = Platform::getFileLastWrittenTime(gameCodeRealPath);

    return game;
}

void unloadGameCode(GameCode& game)
{
    if (game.lib != nullptr)
    {
        Platform::unloadDynamicLib(game.lib);
        game.lib = nullptr;
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

    char directory[256]{};
    Platform::getExeDirectory(directory);
    sprintf(gameCodeRealPath, "%s%s", directory, GAME_DLL_NAME);
    sprintf(gameCodeTempPath, "%s%s", directory, GAME_DLL_NAME_TEMP);

    const auto windowSize = vec2(1280, 720);
    appState.window = Platform::openWindow(windowSize.x, windowSize.y, PROJECT_NAME);
    defer({ Platform::closeWindow(appState.window); });

    Renderer::init(appState.window, windowSize.x, windowSize.y);
    defer({ Renderer::deinit(); });

    auto game = loadGameCode();

    Renderer::addDrawCommand({.mesh = Renderer::Mesh::Triangle, .shader = Renderer::ShaderType::Basic});

    // game.init((void*)&context);

    using namespace Platform;
    using namespace Renderer;
    while (!windowShouldClose && appState.isRunning)
    {
        pollEvents();

        if (isKeyPressed(KeyboardKey::KEY_ESCAPE))
            appState.isRunning = false;

        const auto gameLastWrittenTime = getFileLastWrittenTime(gameCodeRealPath);
        if (gameLastWrittenTime != game.lastWrittenTime)
        {
            // game.preHotReload((void*)&context);
            unloadGameCode(game);
            arenaFreeAll(context.memory);
            std::this_thread::sleep_for(1ms);
            game = loadGameCode();
            // game.postHotReload((void*)&context);
        }

        memset(inputState.keyboard, 0);
        inputState.mouse.leftState = ButtonState::Idle;
        inputState.mouse.rightState = ButtonState::Idle;
        inputState.mouse.pos = {};

        arenaFreeAll(context.tempMemory);

        static vec4 clearColor{0, 0, 0, 1};
        clear(clearColor);
        for (const auto& command : drawCommands)
            draw(command);
        present();
    }
}
