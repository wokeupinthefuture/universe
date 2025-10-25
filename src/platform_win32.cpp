#pragma once

#include "context.hpp"
#include "entity.hpp"
#include "platform.hpp"
#include "input.hpp"

#include <cstring>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Platform
{

static int resolveKeyMapping(KeyboardKey key)
{
    switch (key)
    {
        // Alphabetical keys
        case KeyboardKey::KEY_A: return 'A';
        case KeyboardKey::KEY_B: return 'B';
        case KeyboardKey::KEY_C: return 'C';
        case KeyboardKey::KEY_D: return 'D';
        case KeyboardKey::KEY_E: return 'E';
        case KeyboardKey::KEY_F: return 'F';
        case KeyboardKey::KEY_G: return 'G';
        case KeyboardKey::KEY_H: return 'H';
        case KeyboardKey::KEY_I: return 'I';
        case KeyboardKey::KEY_J: return 'J';
        case KeyboardKey::KEY_K: return 'K';
        case KeyboardKey::KEY_L: return 'L';
        case KeyboardKey::KEY_M: return 'M';
        case KeyboardKey::KEY_N: return 'N';
        case KeyboardKey::KEY_O: return 'O';
        case KeyboardKey::KEY_P: return 'P';
        case KeyboardKey::KEY_Q: return 'Q';
        case KeyboardKey::KEY_R: return 'R';
        case KeyboardKey::KEY_S: return 'S';
        case KeyboardKey::KEY_T: return 'T';
        case KeyboardKey::KEY_U: return 'U';
        case KeyboardKey::KEY_V: return 'V';
        case KeyboardKey::KEY_W: return 'W';
        case KeyboardKey::KEY_X: return 'X';
        case KeyboardKey::KEY_Y: return 'Y';
        case KeyboardKey::KEY_Z: return 'Z';

        // Numerical keys
        case KeyboardKey::KEY_0: return '0';
        case KeyboardKey::KEY_1: return '1';
        case KeyboardKey::KEY_2: return '2';
        case KeyboardKey::KEY_3: return '3';
        case KeyboardKey::KEY_4: return '4';
        case KeyboardKey::KEY_5: return '5';
        case KeyboardKey::KEY_6: return '6';
        case KeyboardKey::KEY_7: return '7';
        case KeyboardKey::KEY_8: return '8';
        case KeyboardKey::KEY_9: return '9';

        // Function keys
        case KeyboardKey::KEY_F1: return VK_F1;
        case KeyboardKey::KEY_F2: return VK_F2;
        case KeyboardKey::KEY_F3: return VK_F3;
        case KeyboardKey::KEY_F4: return VK_F4;
        case KeyboardKey::KEY_F5: return VK_F5;
        case KeyboardKey::KEY_F6: return VK_F6;
        case KeyboardKey::KEY_F7: return VK_F7;
        case KeyboardKey::KEY_F8: return VK_F8;
        case KeyboardKey::KEY_F9: return VK_F9;
        case KeyboardKey::KEY_F10: return VK_F10;
        case KeyboardKey::KEY_F11: return VK_F11;
        case KeyboardKey::KEY_F12: return VK_F12;

        // Modifier keys
        case KeyboardKey::KEY_SHIFT: return VK_SHIFT;
        case KeyboardKey::KEY_CTRL: return VK_CONTROL;
        case KeyboardKey::KEY_ALT: return VK_MENU;
        case KeyboardKey::KEY_META_LEFT: return VK_LWIN;
        case KeyboardKey::KEY_META_RIGHT: return VK_RWIN;
        case KeyboardKey::KEY_CAPS_LOCK: return VK_CAPITAL;

        // Navigation keys
        case KeyboardKey::KEY_UP: return VK_UP;
        case KeyboardKey::KEY_DOWN: return VK_DOWN;
        case KeyboardKey::KEY_LEFT: return VK_LEFT;
        case KeyboardKey::KEY_RIGHT: return VK_RIGHT;
        case KeyboardKey::KEY_HOME: return VK_HOME;
        case KeyboardKey::KEY_END: return VK_END;
        case KeyboardKey::KEY_PAGE_UP: return VK_PRIOR;
        case KeyboardKey::KEY_PAGE_DOWN: return VK_NEXT;

        // Editing keys
        case KeyboardKey::KEY_INSERT: return VK_INSERT;
        case KeyboardKey::KEY_DELETE: return VK_DELETE;
        case KeyboardKey::KEY_BACKSPACE: return VK_BACK;
        case KeyboardKey::KEY_ENTER: return VK_RETURN;
        case KeyboardKey::KEY_TAB: return VK_TAB;
        case KeyboardKey::KEY_ESCAPE: return VK_ESCAPE;
        case KeyboardKey::KEY_SPACE: return VK_SPACE;

        // Punctuation and symbol keys
        case KeyboardKey::KEY_BACKTICK: return VK_OEM_3;
        case KeyboardKey::KEY_MINUS: return VK_OEM_MINUS;
        case KeyboardKey::KEY_EQUAL: return VK_OEM_PLUS;
        case KeyboardKey::KEY_LEFT_BRACKET: return VK_OEM_4;
        case KeyboardKey::KEY_RIGHT_BRACKET: return VK_OEM_6;
        case KeyboardKey::KEY_BACKSLASH: return VK_OEM_5;
        case KeyboardKey::KEY_SEMICOLON: return VK_OEM_1;
        case KeyboardKey::KEY_QUOTE: return VK_OEM_7;
        case KeyboardKey::KEY_COMMA: return VK_OEM_COMMA;
        case KeyboardKey::KEY_PERIOD: return VK_OEM_PERIOD;
        case KeyboardKey::KEY_SLASH: return VK_OEM_2;

        // Special keys
        case KeyboardKey::KEY_PRINT_SCREEN: return VK_SNAPSHOT;
        case KeyboardKey::KEY_PAUSE: return VK_PAUSE;

        default: return 0;
    }
}

static KeyboardKey translateKeyMapping(int vkKey)
{
    switch (vkKey)
    {
        // Alphabetical keys (only uppercase as per Windows VK codes)
        case 'A': return KeyboardKey::KEY_A;
        case 'B': return KeyboardKey::KEY_B;
        case 'C': return KeyboardKey::KEY_C;
        case 'D': return KeyboardKey::KEY_D;
        case 'E': return KeyboardKey::KEY_E;
        case 'F': return KeyboardKey::KEY_F;
        case 'G': return KeyboardKey::KEY_G;
        case 'H': return KeyboardKey::KEY_H;
        case 'I': return KeyboardKey::KEY_I;
        case 'J': return KeyboardKey::KEY_J;
        case 'K': return KeyboardKey::KEY_K;
        case 'L': return KeyboardKey::KEY_L;
        case 'M': return KeyboardKey::KEY_M;
        case 'N': return KeyboardKey::KEY_N;
        case 'O': return KeyboardKey::KEY_O;
        case 'P': return KeyboardKey::KEY_P;
        case 'Q': return KeyboardKey::KEY_Q;
        case 'R': return KeyboardKey::KEY_R;
        case 'S': return KeyboardKey::KEY_S;
        case 'T': return KeyboardKey::KEY_T;
        case 'U': return KeyboardKey::KEY_U;
        case 'V': return KeyboardKey::KEY_V;
        case 'W': return KeyboardKey::KEY_W;
        case 'X': return KeyboardKey::KEY_X;
        case 'Y': return KeyboardKey::KEY_Y;
        case 'Z': return KeyboardKey::KEY_Z;

        // Numerical keys (top row)
        case '0': return KeyboardKey::KEY_0;
        case '1': return KeyboardKey::KEY_1;
        case '2': return KeyboardKey::KEY_2;
        case '3': return KeyboardKey::KEY_3;
        case '4': return KeyboardKey::KEY_4;
        case '5': return KeyboardKey::KEY_5;
        case '6': return KeyboardKey::KEY_6;
        case '7': return KeyboardKey::KEY_7;
        case '8': return KeyboardKey::KEY_8;
        case '9': return KeyboardKey::KEY_9;

        // Function keys
        case VK_F1: return KeyboardKey::KEY_F1;
        case VK_F2: return KeyboardKey::KEY_F2;
        case VK_F3: return KeyboardKey::KEY_F3;
        case VK_F4: return KeyboardKey::KEY_F4;
        case VK_F5: return KeyboardKey::KEY_F5;
        case VK_F6: return KeyboardKey::KEY_F6;
        case VK_F7: return KeyboardKey::KEY_F7;
        case VK_F8: return KeyboardKey::KEY_F8;
        case VK_F9: return KeyboardKey::KEY_F9;
        case VK_F10: return KeyboardKey::KEY_F10;
        case VK_F11: return KeyboardKey::KEY_F11;
        case VK_F12: return KeyboardKey::KEY_F12;

        // Modifier keys
        case VK_SHIFT: return KeyboardKey::KEY_SHIFT;
        case VK_CONTROL: return KeyboardKey::KEY_CTRL;
        case VK_MENU: return KeyboardKey::KEY_ALT;
        case VK_LWIN: return KeyboardKey::KEY_META_LEFT;
        case VK_RWIN: return KeyboardKey::KEY_META_RIGHT;
        case VK_CAPITAL: return KeyboardKey::KEY_CAPS_LOCK;

        // Navigation keys
        case VK_UP: return KeyboardKey::KEY_UP;
        case VK_DOWN: return KeyboardKey::KEY_DOWN;
        case VK_LEFT: return KeyboardKey::KEY_LEFT;
        case VK_RIGHT: return KeyboardKey::KEY_RIGHT;
        case VK_HOME: return KeyboardKey::KEY_HOME;
        case VK_END: return KeyboardKey::KEY_END;
        case VK_PRIOR: return KeyboardKey::KEY_PAGE_UP;
        case VK_NEXT: return KeyboardKey::KEY_PAGE_DOWN;

        // Editing keys
        case VK_INSERT: return KeyboardKey::KEY_INSERT;
        case VK_DELETE: return KeyboardKey::KEY_DELETE;
        case VK_BACK: return KeyboardKey::KEY_BACKSPACE;
        case VK_RETURN: return KeyboardKey::KEY_ENTER;
        case VK_TAB: return KeyboardKey::KEY_TAB;
        case VK_ESCAPE: return KeyboardKey::KEY_ESCAPE;
        case VK_SPACE: return KeyboardKey::KEY_SPACE;

        // Punctuation and symbol keys (using OEM codes)
        case VK_OEM_3: return KeyboardKey::KEY_BACKTICK;
        case VK_OEM_MINUS: return KeyboardKey::KEY_MINUS;
        case VK_OEM_PLUS: return KeyboardKey::KEY_EQUAL;
        case VK_OEM_4: return KeyboardKey::KEY_LEFT_BRACKET;
        case VK_OEM_6: return KeyboardKey::KEY_RIGHT_BRACKET;
        case VK_OEM_5: return KeyboardKey::KEY_BACKSLASH;
        case VK_OEM_1: return KeyboardKey::KEY_SEMICOLON;
        case VK_OEM_7: return KeyboardKey::KEY_QUOTE;
        case VK_OEM_COMMA: return KeyboardKey::KEY_COMMA;
        case VK_OEM_PERIOD: return KeyboardKey::KEY_PERIOD;
        case VK_OEM_2: return KeyboardKey::KEY_SLASH;

        // Special keys
        case VK_SNAPSHOT: return KeyboardKey::KEY_PRINT_SCREEN;
        case VK_PAUSE: return KeyboardKey::KEY_PAUSE;

        default: return KeyboardKey::KEY_UNKNOWN;
    }
}

static vec2 getMonitorSize()
{
    const auto xScreen = GetSystemMetrics(SM_CXSCREEN);
    const auto yScreen = GetSystemMetrics(SM_CYSCREEN);
    return {xScreen, yScreen};
}

static vec2 getWindowSize(HWND hwnd)
{
    vec2 size;

    RECT rect;
    GetClientRect(hwnd, &rect);
    size = {(float)rect.right - (float)rect.left, (float)rect.bottom - (float)rect.top};

    return size;
}

static auto WINAPI windowCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    ENSURE(g_context);

    if (g_context->platform.guiWindowEventCallback)
        if (((WNDPROC)g_context->platform.guiWindowEventCallback)(hwnd, uMsg, wParam, lParam))
            return true;

    switch (uMsg)
    {
        case WM_RBUTTONDOWN:
        {
            g_context->input.mouse.rightState = ButtonState::Pressed;
            break;
        }
        case WM_RBUTTONUP:
        {
            g_context->input.mouse.rightState = ButtonState::Released;
            break;
        }
        case WM_LBUTTONDOWN:
        {
            g_context->input.mouse.leftState = ButtonState::Pressed;
            break;
        }
        case WM_LBUTTONUP:
        {
            g_context->input.mouse.leftState = ButtonState::Released;
            break;
        }
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            auto key = (int)wParam;
            auto wasDown = ((lParam & (1 << 30)) != 0);
            auto isDown = ((lParam & (1 << 31)) == 0);

            if (isDown)
            {
                if (wasDown != isDown)
                    g_context->input.keyboard[translateKeyMapping(key)] = ButtonState::Pressed;
                else
                    g_context->input.keyboard[translateKeyMapping(key)] = ButtonState::Holding;
            }
            else
            {
                g_context->input.keyboard[translateKeyMapping(key)] = ButtonState::Released;
            }
            break;
        }
        case WM_MOUSEMOVE:
        {
            // if (!inputState.mouse.isCaptured)
            // SetCapture(hwnd);
            vec2 mousePos = {(float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam)};
            g_context->input.mouse.delta = g_context->input.mouse.pos - mousePos;
            g_context->input.mouse.pos = mousePos;
            break;
        }
        case WM_KILLFOCUS:
        {
            // if (inputState.mouse.isCaptured)
            //    ReleaseCapture();
            break;
        }

        case WM_CLOSE:
        {
            PostQuitMessage(0);
            g_context->platform.windowShouldClose = true;
            break;
        }
        case WM_SIZE:
        {
            g_context->platform.lastScreenSize = getWindowSize(hwnd);
            break;
        }

        default:
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    return 0;
};

Window openWindow(int width, int height, const char* name)
{

    const HINSTANCE hInst = GetModuleHandle(nullptr);

    WNDCLASSEX wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = windowCallback;
    wc.hInstance = hInst;
    wc.lpszClassName = name;

    RegisterClassEx(&wc);

    const auto hwnd = CreateWindowEx(
        0, name, name, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, hInst, nullptr);

    BOOL USE_DARK_MODE = true;
    DwmSetWindowAttribute(hwnd, 20, &USE_DARK_MODE, sizeof(USE_DARK_MODE));

    const auto monitorEnumCb = [](HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) -> BOOL
    {
        MONITORINFOEX mi;
        mi.cbSize = sizeof(MONITORINFOEX);

        if (GetMonitorInfo(hMonitor, &mi))
        {
            if (mi.dwFlags & MONITORINFOF_PRIMARY && strstr(mi.szDevice, "2") == 0)
                *(bool*)dwData = true;
        }

        return TRUE;
    };

    bool isSecondMonitor = false;
    EnumDisplayMonitors(NULL, NULL, monitorEnumCb, (LPARAM)&isSecondMonitor);

    const auto screenSize = getMonitorSize();
    const auto windowSize = getWindowSize(hwnd);

    static constexpr auto offset = 250;
    vec2 windowPos = {(screenSize.x * 0.5f * (isSecondMonitor ? 1 : -1)) - windowSize.x * 0.5f - offset,
        screenSize.y * 0.5 - windowSize.y * 0.5f};
    SetWindowPos(hwnd, HWND_TOP, windowPos.x, windowPos.y, width, height, SWP_SHOWWINDOW);
    const auto consoleHwnd = GetConsoleWindow();
    SetWindowPos(consoleHwnd,
        HWND_BOTTOM,
        windowPos.x + windowSize.x,
        windowPos.y,
        windowSize.x * 0.5 - offset * 0.5,
        height,
        SWP_SHOWWINDOW);

    return hwnd;
}

void closeWindow(Window window)
{
    DestroyWindow((HWND)window);
}

void pollEvents()
{
    MSG message{};
    while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

void* allocMemory(size_t size, void* startAddr)
{
    return VirtualAlloc(startAddr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
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

float getDpi()
{
    UINT xdpi = 96;
    const auto dc = ::GetDC(nullptr);
    if (dc)
    {
        xdpi = ::GetDeviceCaps(dc, LOGPIXELSX);
        ::ReleaseDC(nullptr, dc);
    }
    return xdpi / 96.f;
}

void* loadDynamicLib(const char* libName)
{
    return LoadLibrary(libName);
}

void unloadDynamicLib(void* lib)
{
    FreeLibrary((HMODULE)lib);
}

void* loadDynamicFunc(void* lib, const char* funcName)
{
    return (void*)GetProcAddress((HMODULE)lib, funcName);
}

static String cloneNameFromPath(const char* path, Arena& cloneMemory, Arena& tempMemory)
{
    const auto sPath = strClone(path, tempMemory);

    const auto nameWithExt = strFindReverse(sPath, strFromLiteral("\\"), false, 1);
    ENSURE(nameWithExt);
    const auto ext = strFind(nameWithExt, strFromLiteral("."), false);
    ENSURE(ext);

    String name;
    name.data = nameWithExt.data;
    name.length = nameWithExt.length - ext.length;

    return strClone(name, cloneMemory);
}

Asset loadAsset(const char* path, AssetType type, Arena& permanentMemory, Arena& tempMemory)
{
    Asset asset{};

    const auto file = CreateFile(path,                    // file to open
        GENERIC_READ,                                     // open for reading
        FILE_SHARE_READ,                                  // share for reading
        NULL,                                             // default security
        OPEN_EXISTING,                                    // existing file only
        FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY,  // normal file
        NULL);                                            // no attr. template
    ENSURE(file != INVALID_HANDLE_VALUE);
    defer({ CloseHandle(file); });

    DWORD fileSize = GetFileSize(file, nullptr);

    switch (type)
    {
        default: LOGIC_ERROR();
        case AssetType::ObjMesh:
        {
            auto buffer = arenaAlloc(permanentMemory, fileSize + 1, alignof(u8));

            DWORD bytesRead{};
            const auto readResult = ReadFile(file, buffer, fileSize, &bytesRead, nullptr);
            ENSURE(readResult != FALSE && bytesRead == fileSize);

            asset.data = (u8*)buffer;
            asset.name = cloneNameFromPath(path, permanentMemory, tempMemory);
            asset.size = fileSize;
            asset.type = type;

            break;
        }
        case AssetType::Texture:
        {
            i32 w{}, h{}, channels{};
            const auto buffer = stbi_load(path, &w, &h, &channels, 4);
            ENSURE(buffer);

            asset.data = buffer;
            asset.name = cloneNameFromPath(path, permanentMemory, tempMemory);
            asset.size = fileSize;
            asset.type = type;

            asset.textureWidth = w;
            asset.textureHeight = h;
            asset.textureChannels = channels;

            break;
        }
    }

    *(char*)(asset.data + asset.size) = '\0';

    return asset;
}

void forEachFileInDirectory(const char* directory, void (*callback)(const char*))
{
    const auto isValidFile = [](WIN32_FIND_DATA& ffd)
    {
        return !(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY || strcmp(ffd.cFileName, ".") == 0 ||
                 strcmp(ffd.cFileName, "..") == 0);
    };

    char directoryWildcard[256]{};
    sprintf(directoryWildcard, "%s\\*", directory);

    WIN32_FIND_DATA ffd{};
    const auto file = FindFirstFile(directoryWildcard, &ffd);
    if (file == INVALID_HANDLE_VALUE)
        return;

    if (isValidFile(ffd))
        callback(ffd.cFileName);
    do
    {
        if (isValidFile(ffd))
            callback(ffd.cFileName);
    } while (FindNextFile(file, &ffd) != 0);
}

}  // namespace Platform
