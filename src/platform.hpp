#pragma once

#include "common/common.hpp"

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
            _logVerbose = true;                                                   \
            logError("platform call failed: " #expr ", hresult: 0x%08x", result); \
            _logVerbose = false;                                                  \
            assert(false);                                                        \
        }                                                                         \
    } while (0)
#define LOGIC_ERROR()             \
    do                            \
    {                             \
        _logVerbose = true;       \
        logError("logic error!"); \
        _logVerbose = false;      \
        assert(false);            \
    } while (0)
#define ENSURE(x)                            \
    do                                       \
    {                                        \
        if (!(x))                            \
        {                                    \
            _logFile = __FILE__;             \
            _logLine = __LINE__;             \
            _logError("ensure failed: " #x); \
            assert(false);                   \
        }                                    \
    } while (0)
#endif

struct PlatformToGameBuffer
{
    void* window;
    bool windowShouldClose;
    struct InputState* input;
    float dpi;
    vec2 lastScreenSize;
    vec2 screenSize;
    void* guiWindowEventCallback;
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

float getDpi();

void* loadDynamicLib(const char* dllName);
void unloadDynamicLib(void* lib);
void* loadDynamicFunc(void* dll, const char* funcName);
#define Platform_loadDynamicFunc(lib, funcName) decltype (&funcName)(Platform::loadDynamicFunc((lib), (#funcName)))

void setInternalPointer(PlatformToGameBuffer& buffer, InputState& input);

}  // namespace Platform
