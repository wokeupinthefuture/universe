#pragma once

#include "platform.hpp"

void guiInit(void** outWindowEventCallback, void* window, float dpi);
void guiBegin();
void guiDraw();
void guiDeinit();
bool guiIsCapturingMouse();
bool guiIsCapturingKeyboard();

struct GuiState
{
    void* selectedEntity;
};
