#include "context.hpp"
#include "game.hpp"

#include "platform_win32.cpp"

#include <chrono>
#include <thread>

using namespace std::chrono_literals;

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
    decltype(&gameUpdateAndRender) updateAndRender;
    decltype(&gamePreHotReload) preHotReload;
    decltype(&gamePostHotReload) postHotReload;
    decltype(&gameExit) exit;
};

GameCode loadGameCode()
{
    GameCode game{};

    Platform::copyFile(gameCodeRealPath, gameCodeTempPath);
    game.lib = Platform::loadDynamicLib(GAME_DLL_NAME_TEMP);
    std::this_thread::sleep_for(1ms);
    game.init = Platform_loadDynamicFunc(game.lib, gameInit);
    game.updateAndRender = Platform_loadDynamicFunc(game.lib, gameUpdateAndRender);
    game.preHotReload = Platform_loadDynamicFunc(game.lib, gamePreHotReload);
    game.postHotReload = Platform_loadDynamicFunc(game.lib, gamePostHotReload);
    game.exit = Platform_loadDynamicFunc(game.lib, gameExit);
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
    game.updateAndRender = nullptr;
    game.preHotReload = nullptr;
    game.postHotReload = nullptr;
    game.init = nullptr;
    game.exit = nullptr;
    game.isLoaded = false;
}

int main()
{
    Context context{};
    contextInit(context, Megabytes(100), Megabytes(4));
    defer({ contextDeinit(context); });

    char directory[256]{};
    Platform::getExeDirectory(directory);
    sprintf(gameCodeRealPath, "%s%s", directory, GAME_DLL_NAME);
    sprintf(gameCodeTempPath, "%s%s", directory, GAME_DLL_NAME_TEMP);

    context.initialWindowSize = vec2(1280, 720);
    context.render.screenSize = context.initialWindowSize;
    Platform::data.screenSize = context.initialWindowSize;
    Platform::data.input = &context.input;
    context.window = Platform::openWindow(context.initialWindowSize.x, context.initialWindowSize.y, PROJECT_NAME);
    defer({ Platform::closeWindow(context.window); });

    auto game = loadGameCode();

    game.init(context);

    while (!Platform::data.windowShouldClose && !context.wantsToQuit)
    {
        Platform::pollEvents();

        if ((i32)Platform::data.lastScreenSize.x != (i32)Platform::data.screenSize.x ||
            (i32)Platform::data.lastScreenSize.y != (i32)Platform::data.screenSize.y)
        {
            context.render.screenSize = Platform::data.screenSize;
            context.render.needsToResize = true;
        }
        Platform::data.lastScreenSize = Platform::data.screenSize;

        const auto gameLastWrittenTime = Platform::getFileLastWrittenTime(gameCodeRealPath);
        if (gameLastWrittenTime != game.lastWrittenTime || context.wantsToReload)
        {
            game.preHotReload(context);

            std::this_thread::sleep_for(1ms);

            unloadGameCode(game);

            arrayClear(context.entityManager.entities);
            arrayClear(context.render.drawCommands);
            arenaFreeAll(context.permanentMemory);
            arenaFreeAll(context.tempMemory);

            std::this_thread::sleep_for(1ms);

            context.wantsToReload = false;

            game = loadGameCode();
            game.postHotReload(context);
        }

        game.updateAndRender(context);

        arenaFreeAll(context.tempMemory);
    }

    game.exit(context);
}
