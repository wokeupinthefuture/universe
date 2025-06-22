#pragma once

#include "platform.hpp"
#include "lib/log.hpp"

#include <cstring>

#define _WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace Platform
{

void* allocMemory(size_t size)
{
    return VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

void freeMemory(void* addr, size_t size)
{
    VirtualFree(addr, 0, MEM_RELEASE);
}

void getExeDirectory(char* outDirectory)
{
    char exePath[MAX_PATH]{};
    GetModuleFileName(nullptr, exePath, MAX_PATH);
    const auto directoryEnd = (intptr_t)std::strrchr(exePath, '\\') + 1 - (intptr_t)exePath;
    std::memcpy(outDirectory, exePath, directoryEnd);
}

u64 getFileLastWrittenTime(const char* filePath)
{
    u64 time{};
    WIN32_FIND_DATA data{};

    const auto handle = FindFirstFile(filePath, &data);
    if (handle != INVALID_HANDLE_VALUE)
    {
        time = ((u64)data.ftLastWriteTime.dwHighDateTime << 32) | (u64)data.ftLastWriteTime.dwLowDateTime;
        FindClose(handle);
        return time;
    }
    else
    {
        logError("file %s not found!", filePath);
    }

    return time;
}

void copyFile(const char* src, const char* dst)
{
    CopyFile(src, dst, false);
}

void* loadDll(const char* dllName)
{
    return LoadLibrary(dllName);
}

void unloadDll(void* dll)
{
    FreeLibrary((HMODULE)dll);
}

void* importDllFunc(void* dll, const char* funcName)
{
    return (void*)GetProcAddress((HMODULE)dll, funcName);
}

}  // namespace Platform
