#include "input.hpp"
#include "context.hpp"

bool isKeyPressed(KeyboardKey key)
{
    ENSURE(g_context);
    return g_context->input.keyboard[key] == ButtonState::Pressed || g_context->input.keyboard[key] == ButtonState::Holding;
}

bool wasKeyPressed(KeyboardKey key)
{
    ENSURE(g_context);
    return g_context->input.keyboard[key] == ButtonState::Pressed;
}

bool isMousePressed(bool left)
{
    ENSURE(g_context);
    const auto btnState = (left ? g_context->input.mouse.leftState : g_context->input.mouse.rightState);
    return btnState == ButtonState::Pressed || btnState == ButtonState::Holding;
}

bool wasMousePressed(bool left)
{
    ENSURE(g_context);
    return (left ? g_context->input.mouse.leftState : g_context->input.mouse.rightState) == ButtonState::Pressed;
}

bool wasMouseReleased(bool left)
{
    ENSURE(g_context);
    return (left ? g_context->input.mouse.leftState : g_context->input.mouse.rightState) == ButtonState::Released;
}
