#pragma once

#include "common/common.hpp"

enum KeyboardKey
{
    // Alphabetical keys
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,

    // Numerical keys
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,

    // Function keys
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,

    // Modifier keys
    KEY_SHIFT,
    KEY_CONTROL_LEFT,
    KEY_CONTROL_RIGHT,
    KEY_ALT_LEFT,
    KEY_ALT_RIGHT,
    KEY_META_LEFT,   // Windows key
    KEY_META_RIGHT,  // Windows key
    KEY_CAPS_LOCK,

    // Navigation keys
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_HOME,
    KEY_END,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,

    // Editing keys
    KEY_INSERT,
    KEY_DELETE,
    KEY_BACKSPACE,
    KEY_ENTER,
    KEY_TAB,
    KEY_ESCAPE,
    KEY_SPACE,

    // Punctuation and symbol keys
    KEY_BACKTICK,       // ` and ~
    KEY_MINUS,          // - and _
    KEY_EQUAL,          // = and +
    KEY_LEFT_BRACKET,   // [ and {
    KEY_RIGHT_BRACKET,  // ] and }
    KEY_BACKSLASH,      // \ and |
    KEY_SEMICOLON,      // ; and :
    KEY_QUOTE,          // ' and "
    KEY_COMMA,          // , and <
    KEY_PERIOD,         // . and >
    KEY_SLASH,          // / and ?

    // Special keys
    KEY_PRINT_SCREEN,
    KEY_PAUSE,
    KEY_UNKNOWN,

    KEY_MAX
};

enum ButtonState
{
    NotPressed,
    Pressed,
    Holding,
    Released
};

struct InputState
{
    struct MouseState
    {
        ButtonState leftState;
        ButtonState rightState;
        vec2 pos;
        vec2 delta;
        bool isCaptured;
    } mouse;

    ButtonState keyboard[(int)KeyboardKey::KEY_MAX];
    ButtonState prevKeyboard[(int)KeyboardKey::KEY_MAX];
};

void setInternalPointer(InputState* state);
bool isKeyPressed(KeyboardKey key);
bool wasKeyPressed(KeyboardKey key);
bool wasKeyReleased(KeyboardKey key);
bool isMousePressed(bool left);
bool wasMousePressed(bool left);
bool wasMouseReleased(bool left);
