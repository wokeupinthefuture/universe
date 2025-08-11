#pragma once

#include <cstdio>
#include <ctime>

inline const char* _logFile = nullptr;
inline int _logLine = 0;
inline bool _logVerbose = false;

#define logInfo(msg, ...)           \
    do                              \
    {                               \
        _logFile = __FILE__;        \
        _logLine = __LINE__;        \
        _logInfo(msg, __VA_ARGS__); \
    } while (0)

template <typename... Args>
void _logInfo(const char* message, Args... args)
{
    char buffer[256];

    const auto time = std::time(nullptr);
    const auto localtime = std::localtime(&time);

    char hours[256];
    std::sprintf(hours, localtime->tm_hour > 9 ? "%i" : "0%i", localtime->tm_hour);
    char minutes[256];
    std::sprintf(minutes, localtime->tm_min > 9 ? "%i" : "0%i", localtime->tm_min);
    char seconds[256];
    std::sprintf(seconds, localtime->tm_sec > 9 ? "%i" : "0%i", localtime->tm_sec);

    if (_logVerbose)
    {
        std::sprintf(buffer, "[%s:%s:%s][Info][%s:%i] %s\n", hours, minutes, seconds, _logFile, _logLine, message);
        std::printf(buffer, args...);
    }
    else
    {
        std::sprintf(buffer, "%s\n", message);
        std::printf(buffer, args...);
    }
}

#define logError(msg, ...)           \
    do                               \
    {                                \
        _logFile = __FILE__;         \
        _logLine = __LINE__;         \
        _logError(msg, __VA_ARGS__); \
    } while (0)

template <typename... Args>
void _logError(const char* message, Args... args)
{
    char buffer[256];

    const auto time = std::time(nullptr);
    const auto localtime = std::localtime(&time);

    char hours[256];
    std::sprintf(hours, localtime->tm_hour > 9 ? "%i" : "0%i", localtime->tm_hour);
    char minutes[256];
    std::sprintf(minutes, localtime->tm_min > 9 ? "%i" : "0%i", localtime->tm_min);
    char seconds[256];
    std::sprintf(seconds, localtime->tm_sec > 9 ? "%i" : "0%i", localtime->tm_sec);

    if (_logVerbose)
    {
        std::sprintf(buffer, "[%s:%s:%s][Error][%s:%i] %s\n", hours, minutes, seconds, _logFile, _logLine, message);
        std::printf(buffer, args...);
    }
    else
    {
        std::sprintf(buffer, "%s\n", message);
        std::printf(buffer, args...);
    }
}
