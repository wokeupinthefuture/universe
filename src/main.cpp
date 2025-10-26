#include "context.hpp"
#include "platform.hpp"
#include "context.cpp"
#include "game.hpp"

#include "common/string.cpp"
#include "common/memory.cpp"
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

void loadAssetsByType(AssetType type)
{
    static AssetType assetLoadType;
    assetLoadType = type;
    Platform::forEachFileInDirectory(ASSETS_PATH[(i32)assetLoadType],
        [](const char* fileName)
        {
            if (assetLoadType == AssetType::ObjMesh && !strstr(fileName, ".obj"))
                return;
            char filePath[256]{};
            sprintf(filePath, "%s\\%s", ASSETS_PATH[(i32)assetLoadType], fileName);
            arrayPush(g_context->platform.assets[(i32)assetLoadType],
                Platform::loadAsset(filePath, assetLoadType, g_context->platformMemory, g_context->tempMemory));
        });
}

int main()
{
    Context context{};
    contextInit(context, Megabytes(100), Megabytes(25));
    defer({ contextDeinit(context); });

    g_context = &context;

    char directory[256]{};
    Platform::getExeDirectory(directory);
    sprintf(gameCodeRealPath, "%s%s", directory, GAME_DLL_NAME);
    sprintf(gameCodeTempPath, "%s%s", directory, GAME_DLL_NAME_TEMP);

    static constexpr vec2 INITIAL_WINDOW_SIZE = vec2(1280, 720);
    context.platform.lastScreenSize = INITIAL_WINDOW_SIZE;
    context.platform.window = Platform::openWindow(INITIAL_WINDOW_SIZE.x, INITIAL_WINDOW_SIZE.y, PROJECT_NAME);
    context.platform.dpi = Platform::getDpi();
    context.render.screenSize = INITIAL_WINDOW_SIZE;
    defer({ Platform::closeWindow(context.platform.window); });

    loadAssetsByType(AssetType::ObjMesh);
    loadAssetsByType(AssetType::Texture);
    loadAssetsByType(AssetType::SkyboxTexture);

    auto game = loadGameCode();

    game.init(context);

    while (!context.platform.windowShouldClose && !context.wantsToQuit)
    {
        Platform::pollEvents();

        if (context.platform.lastScreenSize.x != 0 || context.platform.lastScreenSize.y != 0)
        {
            context.render.screenSize = context.platform.lastScreenSize;
            context.render.needsToResize = true;
            context.platform.lastScreenSize = {};
        }

        const auto gameLastWrittenTime = Platform::getFileLastWrittenTime(gameCodeRealPath);
        if (gameLastWrittenTime != game.lastWrittenTime || context.wantsToReload)
        {
            game.preHotReload(context);

            std::this_thread::sleep_for(1ms);

            contextHotReload(context);
            unloadGameCode(game);

            std::this_thread::sleep_for(1ms);

            context.wantsToReload = false;

            game = loadGameCode();
            game.postHotReload(context);
        }

        game.updateAndRender(context);

        arenaClear(context.tempMemory);
    }

    game.exit(context);
}
