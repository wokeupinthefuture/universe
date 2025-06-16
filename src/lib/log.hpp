#pragma once

#include <cstdio>
#include <ctime>

// #define LOG_VERBOSE

inline const char* _logFile = nullptr;
inline int _logLine = 0;

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

#if defined LOG_VERBOSE
    std::sprintf(
        buffer.data, "[%s:%s:%s][Info][%s:%i] %s\n", hours.data, minutes.data, seconds.data, _logFile, _logLine, message);
    std::printf(buffer.data, args...);
#else
    std::sprintf(buffer, "%s\n", message);
    std::printf(buffer, args...);
#endif
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

#if defined LOG_VERBOSE
    std::sprintf(
        buffer.data, "[%s:%s:%s][Error][%s:%i] %s\n", hours.data, minutes.data, seconds.data, _logFile, _logLine, message);
    std::printf(buffer.data, args...);
#else
    std::sprintf(buffer, "%s\n", message);
    std::printf(buffer, args...);
#endif
}
