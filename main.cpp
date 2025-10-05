#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dwmapi.h>
#include <iostream>
#include <chrono>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwmapi.lib")

#define HOTKEY_TOGGLE 0x48 // 'H'

bool zoomActive = false;
float zoomScale = 2.0f;
bool followMouse = true;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_HOTKEY:
        if (wParam == 1)
            zoomActive = !zoomActive;
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    // Регистрация окна
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WndProc,
                      0, 0, hInstance, nullptr, nullptr, nullptr, nullptr,
                      L"ZoomOverlay", nullptr };
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
        L"ZoomOverlay", L"ZoomOverlay",
        WS_POPUP, 0, 0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        nullptr, nullptr, hInstance, nullptr);

    // Прозрачность
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
    ShowWindow(hwnd, SW_SHOW);
    RegisterHotKey(hwnd, 1, 0, HOTKEY_TOGGLE);

    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    MSG msg;
    while (true)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                return 0;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!zoomActive)
        {
            Sleep(50);
            continue;
        }

        POINT mouse;
        GetCursorPos(&mouse);

        int zoomSize = 200;
        int x = mouse.x - zoomSize;
        int y = mouse.y - zoomSize;
        int w = zoomSize * 2;
        int h = zoomSize * 2;

        HDC hdcTemp = CreateCompatibleDC(hdcScreen);
        HBITMAP hbm = CreateCompatibleBitmap(hdcScreen, w, h);
        SelectObject(hdcTemp, hbm);

        BitBlt(hdcTemp, 0, 0, w, h, hdcScreen, x, y, SRCCOPY);

        // Масштабирование (ZoomScale)
        HDC hdcTarget = GetDC(hwnd);
        SetStretchBltMode(hdcTarget, HALFTONE);
        StretchBlt(hdcTarget,
                   x - (int)(w * (zoomScale - 1) / 2),
                   y - (int)(h * (zoomScale - 1) / 2),
                   (int)(w * zoomScale),
                   (int)(h * zoomScale),
                   hdcTemp, 0, 0, w, h, SRCCOPY);

        ReleaseDC(hwnd, hdcTarget);
        DeleteObject(hbm);
        DeleteDC(hdcTemp);

        Sleep(10);
    }

    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);
    return 0;
}
