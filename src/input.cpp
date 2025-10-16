#include "input.hpp"
#include "platform.hpp"

static InputState* state;

void setInternalPointer(InputState& _state)
{
    state = &_state;
}

bool isKeyPressed(KeyboardKey key)
{
    ENSURE(state);
    return state->keyboard[key] == ButtonState::Pressed || state->keyboard[key] == ButtonState::Holding;
}

bool wasKeyPressed(KeyboardKey key)
{
    ENSURE(state);
    return state->keyboard[key] == ButtonState::Pressed;
}

bool isMousePressed(bool left)
{
    ENSURE(state);
    const auto btnState = (left ? state->mouse.leftState : state->mouse.rightState);
    return btnState == ButtonState::Pressed || btnState == ButtonState::Holding;
}

bool wasMousePressed(bool left)
{
    ENSURE(state);
    return (left ? state->mouse.leftState : state->mouse.rightState) == ButtonState::Pressed;
}

bool wasMouseReleased(bool left)
{
    ENSURE(state);
    return (left ? state->mouse.leftState : state->mouse.rightState) == ButtonState::Released;
}
