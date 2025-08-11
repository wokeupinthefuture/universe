#pragma once

#include "lib/common.hpp"
#include "input.hpp"

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
#endif

namespace Platform
{

using Window = void*;
Window openWindow(int width, int height, const char* name);
void closeWindow(Window window);

inline bool windowShouldClose;

void pollEvents();
void swapBuffers();

// memory
void* allocMemory(size_t size);
void freeMemory(void* addr, size_t size);

// file ops
void getExeDirectory(char* path);
u64 getFileLastWrittenTime(const char* fileName);
void copyFile(const char* src, const char* dst);

void* loadDynamicLib(const char* dllName);
void unloadDynamicLib(void* lib);
void* loadDynamicFunc(void* dll, const char* funcName);
#define Platform_loadDynamicFunc(lib, funcName) decltype (&funcName)(Platform::loadDynamicFunc((lib), (#funcName)))

}  // namespace Platform
