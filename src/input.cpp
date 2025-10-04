#include "input.hpp"

bool isKeyPressed(KeyboardKey key)
{
    return g_input->keyboard[key] == ButtonState::Pressed || g_input->keyboard[key] == ButtonState::Holding;
}

bool wasKeyPressed(KeyboardKey key)
{
    return g_input->keyboard[key] == ButtonState::Pressed;
}

bool isMousePressed(bool left)
{
    const auto state = (left ? g_input->mouse.leftState : g_input->mouse.rightState);
    return state == ButtonState::Pressed || state == ButtonState::Holding;
}

bool wasMousePressed(bool left)
{
    return (left ? g_input->mouse.leftState : g_input->mouse.rightState) == ButtonState::Pressed;
}

bool wasMouseReleased(bool left)
{
    return (left ? g_input->mouse.leftState : g_input->mouse.rightState) == ButtonState::Released;
}
