#pragma once

#include <cstdio>
#include <ctime>

#include "utils.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>

inline const char* g_logFile = nullptr;
inline int g_logLine = 0;

enum class LogFlag
{
    Verbose = 1 << 0,
    Error = 1 << 1,
    WideChar = 1 << 2
};

DEFINE_ENUM_BITWISE_OPERATORS(LogFlag)

inline LogFlag g_logFlags = LogFlag::Verbose;

#define logInfo(msg, ...)        \
    do                           \
    {                            \
        g_logFile = __FILE__;    \
        g_logLine = __LINE__;    \
        g_log(msg, __VA_ARGS__); \
    } while (0)

#define logInfoW(msg, ...)                \
    do                                    \
    {                                     \
        g_logFile = __FILE__;             \
        g_logLine = __LINE__;             \
        g_logFlags |= LogFlag::WideChar;  \
        g_logW(msg, __VA_ARGS__);         \
        g_logFlags &= ~LogFlag::WideChar; \
    } while (0)

#define logError(msg, ...)             \
    do                                 \
    {                                  \
        g_logFile = __FILE__;          \
        g_logLine = __LINE__;          \
        g_logFlags |= LogFlag::Error;  \
        g_log(msg, __VA_ARGS__);       \
        g_logFlags &= ~LogFlag::Error; \
    } while (0)

#define logErrorW(msg, ...)                                \
    do                                                     \
    {                                                      \
        g_logFile = __FILE__;                              \
        g_logLine = __LINE__;                              \
        g_logFlags |= LogFlag::WideChar | LogFlag::Error;  \
        g_logW(msg, __VA_ARGS__);                          \
        g_logFlags &= ~LogFlag::WideChar | LogFlag::Error; \
    } while (0)

template <typename... Args>
void g_log(const char* message, Args... args)
{
    char finalBuffer[1024];
    char buffer[1024];
    if (bool(g_logFlags & LogFlag::Verbose))
    {
        const auto now = time(nullptr);
        const auto localTime = localtime(&now);
        char hours[256];
        sprintf(hours, localTime->tm_hour > 9 ? "%i" : "0%i", localTime->tm_hour);
        char minutes[256];
        sprintf(minutes, localTime->tm_min > 9 ? "%i" : "0%i", localTime->tm_min);
        char seconds[256];
        sprintf(seconds, localTime->tm_sec > 9 ? "%i" : "0%i", localTime->tm_sec);
        sprintf(buffer, message, args...);
        sprintf(finalBuffer,
            "[%s:%s:%s][%s][%s:%i] %s\n",
            hours,
            minutes,
            seconds,
            ((int)g_logFlags & (int)LogFlag::Error) ? "Error" : "Info",
            g_logFile,
            g_logLine,
            buffer);
        OutputDebugStringA(finalBuffer);
    }
    else
    {
        sprintf(buffer, message, args...);
        sprintf(finalBuffer, "%s\n", buffer);
        OutputDebugStringA(finalBuffer);
    }
}

template <typename... Args>
void g_logW(const wchar_t* message, Args... args)
{
    wchar_t finalBuffer[1024];
    wchar_t buffer[1024];
    if (bool(g_logFlags & LogFlag::Verbose))
    {
        const auto now = time(nullptr);
        const auto localTime = localtime(&now);
        wchar_t hours[256];
        swprintf(hours, 256, localTime->tm_hour > 9 ? L"%i" : L"0%i", localTime->tm_hour);
        wchar_t minutes[256];
        swprintf(minutes, 256, localTime->tm_min > 9 ? L"%i" : L"0%i", localTime->tm_min);
        wchar_t seconds[256];
        swprintf(seconds, 256, localTime->tm_sec > 9 ? L"%i" : L"0%i", localTime->tm_sec);
        swprintf(buffer, 1024, message, args...);
        swprintf(finalBuffer,
            1024,
            L"[%s:%s:%s][%s][%S:%i] %s\n",
            hours,
            minutes,
            seconds,
            ((int)g_logFlags & (int)LogFlag::Error) ? L"Error" : L"Info",
            g_logFile,
            g_logLine,
            buffer);
        OutputDebugStringW(finalBuffer);
    }
    else
    {
        swprintf(buffer, 1024, message, args...);
        swprintf(finalBuffer, 1024, L"%s\n", buffer);
        OutputDebugStringW(finalBuffer);
    }
}
