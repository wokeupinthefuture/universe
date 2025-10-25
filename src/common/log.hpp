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
    Error = 1 << 1
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

#define logError(msg, ...)             \
    do                                 \
    {                                  \
        g_logFile = __FILE__;          \
        g_logLine = __LINE__;          \
        g_logFlags |= LogFlag::Error;  \
        g_log(msg, __VA_ARGS__);       \
        g_logFlags &= ~LogFlag::Error; \
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
