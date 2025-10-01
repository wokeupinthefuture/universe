#include "input.hpp"

bool isKeyPressed(KeyboardKey key)
{
    return g_input->keyboard[key] == ButtonState::Pressed || g_input->keyboard[key] == ButtonState::Holding;
}

bool wasKeyPressed(KeyboardKey key)
{
    return g_input->keyboard[key] == ButtonState::Pressed;
}

bool isMouseDown(bool left)
{
    return (left ? g_input->mouse.leftState : g_input->mouse.rightState) == ButtonState::Pressed;
}
