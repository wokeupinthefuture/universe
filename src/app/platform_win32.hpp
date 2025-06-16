#pragma once

#include "lib/common.hpp"
#define _WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace Platform
{

// memory
void* allocMemory(size_t size);
void freeMemory(void* addr, size_t size);

// file ops
void getExeDirectory(char* path);
u64 getFileLastWrittenTime(const char* fileName);
void copyFile(const char* src, const char* dst);

void* loadDll(const char* dllName);
void unloadDll(void* dll);

void* importDllFunc(void* dll, const char* funcName);

#define Platform_importDllFunc(dll, funcName) decltype (&funcName)(Platform::importDllFunc((dll), (#funcName)))

}  // namespace Platform
