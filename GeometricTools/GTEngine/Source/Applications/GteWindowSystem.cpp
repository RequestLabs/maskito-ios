// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Applications/GteWindow.h>
#include <Applications/GteWindowSystem.h>
using namespace gte;

WindowSystem gte::TheWindowSystem;


WindowSystem::~WindowSystem ()
{
    if (mHandleMap.size() == 0 && mAtom)
    {
        UnregisterClass(mWindowClassName, 0);
    }
}

WindowSystem::WindowSystem ()
    :
    mWindowClassName(L"GTEngine1Window"),
    mAtom(0)
{
    WNDCLASS wc;
    wc.style         = CS_OWNDC;
    wc.lpfnWndProc   = WindowProcedure;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = nullptr;
    wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    wc.lpszClassName = mWindowClassName;
    wc.lpszMenuName  = nullptr;
    mAtom = RegisterClass(&wc);
}

bool WindowSystem::GetWindowRectangle (int xClientSize, int yClientSize,
    DWORD style, RECT& windowRectangle)
{
    windowRectangle.left = 0;
    windowRectangle.top = 0;
    windowRectangle.right = (LONG)xClientSize - 1;
    windowRectangle.bottom = (LONG)yClientSize - 1;
    return AdjustWindowRect(&windowRectangle, style, FALSE) != FALSE;
}

void WindowSystem::CreateFrom (Window::Parameters& parameters)
{
    DWORD style;
    if (parameters.allowResize)
    {
        style = WS_OVERLAPPEDWINDOW;
    }
    else
    {
        // This removes WS_THICKFRAME and WS_MAXIMIZEBOX from
        // WS_OVERLAPPEDWINDOW, both of which allow resizing of windows.
        style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    }

    if (parameters.hscrollBar)
    {
        style |= WS_HSCROLL;
    }

    if (parameters.vscrollBar)
    {
        style |= WS_VSCROLL;
    }

    RECT rectangle;
    bool adjusted = GetWindowRectangle(parameters.xSize, parameters.ySize,
        style, rectangle);
    if (adjusted)
    {
        int adjustedXSize = (int)rectangle.right - (int)rectangle.left + 1;
        int adjustedYSize = (int)rectangle.bottom - (int)rectangle.top + 1;
        parameters.handle = CreateWindow(mWindowClassName,
            parameters.title.c_str(), style, parameters.xOrigin,
            parameters.yOrigin, adjustedXSize, adjustedYSize, nullptr,
            nullptr, nullptr, nullptr);

        // AdjustWindowRect decides that scroll bars cover client space,
        // so the adjustment reduces the requested client rectangle size.
        // We have to detect this and recreate a window of the correct size.
        if (parameters.hscrollBar || parameters.vscrollBar)
        {
            GetClientRect(parameters.handle, &rectangle);
            int clientXSize = (int)rectangle.right - (int)rectangle.left;
            int clientYSize = (int)rectangle.bottom - (int)rectangle.top;
            if (clientXSize != parameters.xSize
            ||  clientYSize != parameters.ySize)
            {
                DestroyWindow(parameters.handle);
                adjustedXSize += parameters.xSize - clientXSize;
                adjustedYSize += parameters.ySize - clientYSize;
                parameters.handle = CreateWindow(mWindowClassName,
                    parameters.title.c_str(), style, parameters.xOrigin,
                    parameters.yOrigin, adjustedXSize, adjustedYSize,
                    0, 0, 0, 0);
                GetClientRect(parameters.handle, &rectangle);
            }
        }

#if defined(GTE_DEV_OPENGL)
        parameters.engine = new GL4Engine(parameters.handle, false);
#else
        // TODO: Remove xSize, ySize from constructor.  Query for these
        // from parameters.handle.
        parameters.engine = new DX11Engine(nullptr, parameters.handle,
            parameters.xSize, parameters.ySize, D3D_DRIVER_TYPE_HARDWARE,
            nullptr, parameters.deviceCreationFlags, parameters.featureLevel);
#endif
        if (parameters.engine->GetDevice())
        {
            parameters.created = true;
        }
        else
        {
            LogError("Cannot create graphics engine.");
            delete parameters.engine;
            parameters.engine = nullptr;
            parameters.created = false;
        }
    }
    else
    {
        LogError("AdjustWindowRect failed.");
        parameters.handle = nullptr;
        parameters.engine = nullptr;
        parameters.created = false;
    }
}

void WindowSystem::Extract (LPARAM lParam, int& x, int& y)
{
    x = (int)(short)(lParam & 0xFFFF);
    y = (int)(short)((lParam & 0xFFFF0000) >> 16);
}

void WindowSystem::Extract (WPARAM wParam, int& x, int& y)
{
    x = (int)(short)(wParam & 0xFFFF);
    y = (int)(short)((wParam & 0xFFFF0000) >> 16);
}

LRESULT CALLBACK WindowSystem::WindowProcedure (HWND handle, UINT message,
    WPARAM wParam, LPARAM lParam)
{
    auto iter = TheWindowSystem.mHandleMap.find(handle);
    if (iter == TheWindowSystem.mHandleMap.end())
    {
        return DefWindowProc(handle, message, wParam, lParam);
    }

    Window& window = *iter->second;

    switch (message)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(handle, &ps);

            window.OnDisplay();

            EndPaint(handle, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
        {
            // This tells Windows not to erase the background (and that the
            // application is doing so).
            return 1;
        }
        case WM_MOVE:
        {
            // Get the origin of the moved window.  The y-value for window
            // moves is left-handed.
            int x, y;
            Extract(lParam, x, y);

            window.OnMove(x, y);
            return 0;
        }
        case WM_SIZE:
        {
            // Get the new size of the window.
            int xSize, ySize;
            Extract(lParam, xSize, ySize);

            if (wParam == SIZE_MINIMIZED)
            {
                // assert:  xSize == 0 and ySize == 0
                window.OnMinimize();
            }
            else if (wParam == SIZE_MAXIMIZED)
            {
                window.OnMaximize();
                window.OnResize(xSize, ySize);
            }
            else if (wParam == SIZE_RESTORED)
            {

                window.OnResize(xSize, ySize);
            }
            return 0;
        }
        case WM_CHAR:
        {
            // Get thet translated key code.
            unsigned char key = (unsigned char)(char)wParam;

            // Get the cursor position in client coordinates.
            POINT point;
            GetCursorPos(&point);
            ScreenToClient(handle, &point);
            int x = (int)point.x;
            int y = (int)point.y;

            window.OnCharPress(key, x, y);
            return 0;
        }
        case WM_KEYDOWN:
        {
            // Get the virtual key code.
            int key = (int)wParam;

            // Quit the application when the 'escape' key is pressed.
            if (key == VK_ESCAPE)
            {
                PostQuitMessage(0);
                return 0;
            }

            // Get the cursor position in client coordinates.
            POINT point;
            GetCursorPos(&point);
            ScreenToClient(handle, &point);
            int x = (int)point.x;
            int y = (int)point.y;

            window.OnKeyDown(key, x, y);
            return 0;
        }
        case WM_KEYUP:
        {
            // Get the virtual key code.
            int key = (int)wParam;

            // Get the cursor position in client coordinates.
            POINT point;
            GetCursorPos(&point);
            ScreenToClient(handle, &point);
            int x = (int)point.x;
            int y = (int)point.y;

            window.OnKeyUp(key, x, y);
            return 0;
        }
        case WM_LBUTTONDOWN:
        {
            // Get the modifier flags.
            unsigned int modifiers = (unsigned int)wParam;

            // Get the cursor position in client coordinates.
            int x, y;
            Extract(lParam, x, y);

            window.OnMouseClick(Window::MOUSE_LEFT, Window::MOUSE_DOWN,
                x, y, modifiers);
            return 0;
        }
        case WM_LBUTTONUP:
        {
            // Get the modifier flags.
            unsigned int modifiers = (unsigned int)wParam;

            // Get the cursor position in client coordinates.
            int x, y;
            Extract(lParam, x, y);

            window.OnMouseClick(Window::MOUSE_LEFT, Window::MOUSE_UP,
                x, y, modifiers);
            return 0;
        }
        case WM_MBUTTONDOWN:
        {
            // Get the modifier flags.
            unsigned int modifiers = (unsigned int)wParam;

            // Get the cursor position in client coordinates.
            int x, y;
            Extract(lParam, x, y);

            window.OnMouseClick(Window::MOUSE_MIDDLE, Window::MOUSE_DOWN,
                x, y, modifiers);
            return 0;
        }
        case WM_MBUTTONUP:
        {
            // Get the modifier flags.
            unsigned int modifiers = (unsigned int)wParam;

            // Get the cursor position in client coordinates.
            int x, y;
            Extract(lParam, x, y);

            window.OnMouseClick(Window::MOUSE_MIDDLE, Window::MOUSE_UP,
                x, y, modifiers);
            return 0;
        }
        case WM_RBUTTONDOWN:
        {
            // Get the modifier flags.
            unsigned int modifiers = (unsigned int)wParam;

            // Get the cursor position in client coordinates.
            int x, y;
            Extract(lParam, x, y);

            window.OnMouseClick(Window::MOUSE_RIGHT, Window::MOUSE_DOWN,
                x, y, modifiers);
            return 0;
        }
        case WM_RBUTTONUP:
        {
            // Get the modifier flags.
            unsigned int modifiers = (unsigned int)wParam;

            // Get the cursor position in client coordinates.
            int x, y;
            Extract(lParam, x, y);

            window.OnMouseClick(Window::MOUSE_RIGHT, Window::MOUSE_UP,
                x, y, modifiers);
            return 0;
        }
        case WM_MOUSEMOVE:
        {
            // Get the modifier flags.
            unsigned int modifiers = (unsigned int)wParam;

            // Get the cursor position in client coordinates.
            int x, y;
            Extract(lParam, x, y);

            Window::MouseButton button;
            if (wParam & MK_LBUTTON)
            {
                button = Window::MOUSE_LEFT;
            }
            else if (wParam & MK_MBUTTON)
            {
                button = Window::MOUSE_MIDDLE;
            }
            else if (wParam & MK_RBUTTON)
            {
                button = Window::MOUSE_RIGHT;
            }
            else
            {
                button = Window::MOUSE_NONE;
            }

            window.OnMouseMotion(button, x, y, modifiers);
            return 0;
        }
        case WM_MOUSEWHEEL:
        {
            // Get the modifier flags and the amount the wheel rotated.
            int modifiers, delta;
            Extract(wParam, modifiers, delta);

            // Get the cursor position in client coordinates.
            POINT point;
            GetCursorPos(&point);
            ScreenToClient(handle, &point);
            int x = (int)point.x;
            int y = (int)point.y;

            window.OnMouseWheel(delta, x, y, (unsigned int)modifiers);
            return 0;
        }
        case WM_HSCROLL:  // 0x0114
        case WM_VSCROLL:  // 0x0115
        {
            int bar = message - WM_HSCROLL;  // 0 or 1
            switch (LOWORD(wParam))
            {
            case SB_LINELEFT:
                window.OnScrollDecrementLoRes(bar);
                break;
            case SB_LINERIGHT:
                window.OnScrollIncrementLoRes(bar);
                break;
            case SB_PAGELEFT:
                window.OnScrollDecrementHiRes(bar);
                break;
            case SB_PAGERIGHT:
                window.OnScrollIncrementHiRes(bar);
                break;
            case SB_THUMBPOSITION:
                window.OnScrollEndTracking(bar);
                break;
            case SB_THUMBTRACK:
                window.OnScrollTracking(bar);
                break;
            default:
                // Not handled:  SB_LEFT, SB_RIGHT, SB_ENDSCROLL
                break;
            }
            return 0;
        }
        case WM_CLOSE:
        {
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(handle, message, wParam, lParam);
}

