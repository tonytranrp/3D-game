// Main.cpp
#include "Main.h"
#include <windowsx.h>
// Main.cpp
#include "Main.h"
#include <windowsx.h>

// Global variables
RenderUtils renderer;
bool keys[256] = {};
int lastMouseX = 0;
int lastMouseY = 0;
bool firstMouse = true;
bool mouseCaptured = false;

void ToggleMouseCapture(HWND hwnd, bool enable) {
    if (enable) {
        // Hide cursor and capture mouse
        ShowCursor(FALSE);
        SetCapture(hwnd);

        // Lock cursor to center of window
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        POINT center = {
            (clientRect.right - clientRect.left) / 2,
            (clientRect.bottom - clientRect.top) / 2
        };
        ClientToScreen(hwnd, &center);
        SetCursorPos(center.x, center.y);

        mouseCaptured = true;
    }
    else {
        // Release mouse and show cursor
        ShowCursor(TRUE);
        ReleaseCapture();
        mouseCaptured = false;
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Register the window class
    const wchar_t CLASS_NAME[] = L"Game Window Class";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClass(&wc);

    // Create the window
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"3D Game (Press ESC to toggle mouse capture)",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    // Initialize renderer
    if (!renderer.Initialize(hwnd, 800, 600)) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    // Center mouse position
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    lastMouseX = (clientRect.right - clientRect.left) / 2;
    lastMouseY = (clientRect.bottom - clientRect.top) / 2;

    // Enable mouse capture by default
    ToggleMouseCapture(hwnd, true);

    // Message loop
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // Handle input
            float forward = 0.0f;
            float right = 0.0f;
            float up = 0.0f;

            if (keys['W']) forward += 1.0f;
            if (keys['S']) forward -= 1.0f;
            if (keys['D']) right += 1.0f;
            if (keys['A']) right -= 1.0f;
            if (keys['Q']) up -= 1.0f;
            if (keys['E']) up += 1.0f;

            // Update and render
            renderer.UpdateCamera(forward, right, up, 0.0f, 0.0f);
            renderer.Update(0.0f);
            renderer.Render();

            if (mouseCaptured) {
                // Reset cursor to center
                RECT clientRect;
                GetClientRect(hwnd, &clientRect);
                POINT center = {
                    (clientRect.right - clientRect.left) / 2,
                    (clientRect.bottom - clientRect.top) / 2
                };
                ClientToScreen(hwnd, &center);
                SetCursorPos(center.x, center.y);
            }
        }
    }

    renderer.Cleanup();
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
        keys[wParam & 0xFF] = true;
        if (wParam == VK_ESCAPE) {
            // Toggle mouse capture when ESC is pressed
            ToggleMouseCapture(hwnd, !mouseCaptured);
        }
        return 0;

    case WM_KEYUP:
        keys[wParam & 0xFF] = false;
        return 0;

    case WM_MOUSEMOVE:
        if (mouseCaptured)
        {
            int mouseX = GET_X_LPARAM(lParam);
            int mouseY = GET_Y_LPARAM(lParam);

            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int centerX = (clientRect.right - clientRect.left) / 2;
            int centerY = (clientRect.bottom - clientRect.top) / 2;

            float deltaX = static_cast<float>(mouseX - centerX) * 0.005f;  // Increased sensitivity
            float deltaY = static_cast<float>(mouseY - centerY) * 0.005f;  // Increased sensitivity

            renderer.UpdateCamera(0.0f, 0.0f, 0.0f, deltaY, deltaX);  // Removed negative signs
        }
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}