// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "CameraAndLightNodesWindow.h"

int main(int, char const*[])
{
#if defined(_DEBUG)
    LogReporter reporter(
        "LogReport.txt",
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL);
#endif

    Window::Parameters parameters(
        L"CameraAndLightNodesWindow", 0, 0, 640, 480);

    CameraAndLightNodesWindow* window =
        TheWindowSystem.Create<CameraAndLightNodesWindow>(parameters);

    if (window)
    {
        HWND handle = window->GetHandle();
        ShowWindow(handle, SW_SHOW);
        UpdateWindow(handle);

        for (;;)
        {
            MSG msg;
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    break;
                }

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                if (!window->IsMinimized())
                {
                    window->OnIdle();
                }
            }
        }

        TheWindowSystem.Destroy<CameraAndLightNodesWindow>(window);
    }

    return 0;
}
