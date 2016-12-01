// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Applications/GteWindow.h>

namespace gte
{

class GTE_IMPEXP WindowSystem
{
public:
    // Construction and destruction. This is a singleton class.
    ~WindowSystem ();
    WindowSystem ();

    // Get the true window size for the specified client size.  The true size
    // includes extra space for window decorations (window border, menu bar,
    // and so on).  This information is useful to know before creating a
    // window to ensure the to-be-created window fits within the monitor
    // resolution.
    static bool GetWindowRectangle (int xClientSize, int yClientSize,
        DWORD style, RECT& windowRectangle);

    // Create and destroy windows.  Derived classes may extend the inputs
    // using a nested class derived from Window::Parameters
    template <typename WindowType>
    WindowType* Create (typename WindowType::Parameters& parameters);

    template <typename WindowType>
    void Destroy (WindowType*& window);

private:
    // Window creation and destruction.
    void CreateFrom (Window::Parameters& parameters);

    // Extraction of cursor location, avoiding the extraction in <windows.h>
    // that does not work when you have dual monitors.
    static void Extract (LPARAM lParam, int& x, int& y);
    static void Extract (WPARAM wParam, int& x, int& y);

    // The event handler.
    static LRESULT CALLBACK WindowProcedure (HWND handle, UINT message,
        WPARAM wParam, LPARAM lParam);

    wchar_t const* mWindowClassName;
    ATOM mAtom;
    std::map<HWND,Window*> mHandleMap;
};

extern GTE_IMPEXP WindowSystem TheWindowSystem;


template <typename WindowType>
WindowType* WindowSystem::Create(typename WindowType::Parameters& parameters)
{
    CreateFrom(parameters);
    if (parameters.created)
    {
        WindowType* window = new WindowType(parameters);
        mHandleMap[parameters.handle] = window;
        if (parameters.created)
        {
            return window;
        }
        Destroy(window);
    }
    else
    {
        LogError("WindowSystem::Create failed.");
    }
    return nullptr;
}

template <typename WindowType>
void WindowSystem::Destroy(WindowType*& window)
{
    if (window)
    {
        HWND handle = window->GetHandle();
        mHandleMap.erase(handle);
        delete window;
        window = nullptr;
        DestroyWindow(handle);
    }
}


}
