// Main.h
#pragma once
#include <Windows.h>
#include "../RenderUtils/RenderUtils.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ToggleMouseCapture(HWND hwnd, bool enable);