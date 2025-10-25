#pragma once

#include "common/common.hpp"
#include "common/heap_array.hpp"

#define PLATFORM_WIN32 0
#define PLATFORM_TYPE PLATFORM_WIN32

#if PLATFORM_TYPE == PLATFORM_WIN32

#define GAME_API __declspec(dllexport)
#define HR_ASSERT(expr)                                                           \
    do                                                                            \
    {                                                                             \
        const auto result = expr;                                                 \
        if (FAILED(result))                                                       \
        {                                                                         \
            logError("platform call failed: " #expr ", hresult: 0x%08x", result); \
            assert(false);                                                        \
        }                                                                         \
    } while (0)

#define LOGIC_ERROR()             \
    do                            \
    {                             \
        logError("logic error!"); \
        assert(false);            \
    } while (0)

#define ENSURE(x)                          \
    do                                     \
    {                                      \
        if (!(x))                          \
        {                                  \
            logError("ensure failed:" #x); \
            assert(false);                 \
        }                                  \
    } while (0)
#endif

static constexpr const char* ASSETS_PATH[] = {
    "resources/models/",
    "resources/textures/",
};

enum class AssetType
{
    ObjMesh,
    Texture,
    Max
};

struct Asset
{
    const u8* data;
    String name;
    size_t size;
    AssetType type;
    u32 textureWidth;
    u32 textureHeight;
    u32 textureChannels;
};

struct PlatformToGameBuffer
{
    void* window;
    bool windowShouldClose;
    float dpi;
    vec2 lastScreenSize;
    void* guiWindowEventCallback;
    HeapArray<Asset> assets[(i32)AssetType::Max];
};

namespace Platform
{

using Window = void*;
Window openWindow(int width, int height, const char* name);
void closeWindow(Window window);

void pollEvents();

void* allocMemory(size_t size, void* startAddr = nullptr);
void freeMemory(void* addr, size_t size);

void getExeDirectory(char* path);
u64 getFileLastWrittenTime(const char* fileName);
void copyFile(const char* src, const char* dst);

void forEachFileInDirectory(const char* directory, void (*callback)(const char*));

float getDpi();

void* loadDynamicLib(const char* dllName);
void unloadDynamicLib(void* lib);
void* loadDynamicFunc(void* dll, const char* funcName);
#define Platform_loadDynamicFunc(lib, funcName) decltype (&funcName)(Platform::loadDynamicFunc((lib), (#funcName)))

Asset loadAsset(const char* path, AssetType type, Arena& permanentMemory, Arena& tempMemory);

}  // namespace Platform
