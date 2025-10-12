#include "gui.hpp"
#include "common/log.hpp"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.cpp"
#include "imgui/backends/imgui_impl_dx11.cpp"

void guiInit(void** outWindowEventCallback, void* window, float dpi)
{
    ImGui_ImplWin32_EnableDpiAwareness();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(dpi);  // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing
    style.FontScaleDpi = dpi;  // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We

    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(device.Get(), deviceContext.Get());

    *outWindowEventCallback = (void*)ImGui_ImplWin32_WndProcHandler;
}

void guiBegin()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void guiDraw()
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void guiDeinit()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

bool guiIsCapturingMouse()
{
    auto& io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

bool guiIsCapturingKeyboard()
{
    auto& io = ImGui::GetIO();
    return io.WantCaptureKeyboard;
}
