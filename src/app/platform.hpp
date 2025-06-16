#pragma once

#define PLATFORM_WIN32 0

#define PLATFORM_TYPE PLATFORM_WIN32

#if PLATFORM_TYPE == PLATFORM_WIN32
#include "platform_win32.hpp"
#endif
