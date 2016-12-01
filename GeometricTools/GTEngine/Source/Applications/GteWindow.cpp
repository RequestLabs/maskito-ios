// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Applications/GteWindow.h>
using namespace gte;


Window::~Window()
{
    delete mEngine;
}

Window::Window(Parameters& parameters)
    :
    mHandle(parameters.handle),
    mTitle(parameters.title),
    mXOrigin(parameters.xOrigin),
    mYOrigin(parameters.yOrigin),
    mXSize(parameters.xSize),
    mYSize(parameters.ySize),
    mAllowResize(parameters.allowResize),
    mIsMinimized(false),
    mIsMaximized(false),
    mEngine(parameters.engine)
{
    // If scroll bars are requested, the Window-derived class constructor
    // should call the function Window::SetScrollInterval(...) to set the
    // scroll bar range.  Increments and decrements may be set separately
    // directly via class members.
    mHasScroll[0] = parameters.hscrollBar;
    mHasScroll[1] = parameters.vscrollBar;
    for (int i = 0; i < 2; ++i)
    {
        ZeroMemory(&mScrollInfo[i], sizeof(SCROLLINFO));
        mScrollInfo[i].cbSize = sizeof(SCROLLINFO);
        mScrollInfo[i].fMask = SIF_ALL;
        GetScrollInfo(mHandle, i, &mScrollInfo[i]);
        mScrollLoResDelta[i] = 1;
        mScrollHiResDelta[i] = 1;
    }
}

void Window::SetTitle(std::wstring const& title)
{
    SetWindowText(mHandle, title.c_str());
    mTitle = title;
}

void Window::OnMove(int x, int y)
{
    mXOrigin = x;
    mYOrigin = y;
}

bool Window::OnResize(int xSize, int ySize)
{
    mIsMinimized = false;
    mIsMaximized = false;

    if (xSize != mXSize || ySize != mYSize)
    {
        mXSize = xSize;
        mYSize = ySize;

        if (mEngine)
        {
            mEngine->Resize(xSize, ySize);
        }
        return true;
    }

    return false;
}

void Window::OnMinimize()
{
    mIsMinimized = true;
    mIsMaximized = false;
}

void Window::OnMaximize()
{
    mIsMinimized = false;
    mIsMaximized = true;
}

void Window::OnDisplay()
{
    // Stub for derived classes.
}

void Window::OnIdle()
{
    // Stub for derived classes.
}

bool Window::OnCharPress(unsigned char key, int, int)
{
    if (key == VK_ESCAPE)
    {
        // Quit the application when the 'escape' key is pressed.
        OnClose();
        return true;
    }

    if (key == ' ')
    {
        mTimer.Reset();
        return true;
    }

    return false;
}

bool Window::OnKeyDown(int, int, int)
{
    // Stub for derived classes.
    return false;
}

bool Window::OnKeyUp(int, int, int)
{
    // Stub for derived classes.
    return false;
}

bool Window::OnMouseClick(MouseButton, MouseState, int, int, unsigned int)
{
    // stub for derived classes
    return false;
}

bool Window::OnMouseMotion(MouseButton, int, int, unsigned int)
{
    // stub for derived classes
    return false;
}

bool Window::OnMouseWheel(int, int, int, unsigned int)
{
    // Stub for derived classes.
    return false;
}

void Window::SetMousePosition(int x, int y)
{
    POINT point = { (LONG)x, (LONG)y };
    ClientToScreen(mHandle, &point);
    SetCursorPos(point.x, point.y);
}

void Window::GetMousePosition(int& x, int& y) const
{
    POINT point;
    GetCursorPos(&point);
    ScreenToClient(mHandle, &point);
    x = (int)point.x;
    y = (int)point.y;
}

void Window::SetScrollInterval(int bar, int minValue, int maxValue)
{
    mScrollInfo[bar].fMask = SIF_RANGE;
    mScrollInfo[bar].nMin = minValue;
    mScrollInfo[bar].nMax = maxValue;
    SetScrollInfo(mHandle, bar, &mScrollInfo[bar], FALSE);
}

void Window::GetScrollInterval(int bar, int& minValue, int& maxValue) const
{
    mScrollInfo[bar].fMask = SIF_RANGE;
    GetScrollInfo(mHandle, bar, &mScrollInfo[bar]);
    minValue = mScrollInfo[bar].nMin;
    maxValue = mScrollInfo[bar].nMax;
}

int Window::SetScrollPosition(int bar, int value)
{
    mScrollInfo[bar].fMask = SIF_POS;
    mScrollInfo[bar].nPos = value;
    return SetScrollInfo(mHandle, bar, &mScrollInfo[bar], FALSE);
}

int Window::GetScrollPosition(int bar) const
{
    mScrollInfo[bar].fMask = SIF_POS;
    GetScrollInfo(mHandle, bar, &mScrollInfo[bar]);
    return mScrollInfo[bar].nPos;
}

int Window::OnScrollIncrementLoRes(int bar)
{
    SCROLLINFO& info = mScrollInfo[bar];
    info.fMask = SIF_POS;
    GetScrollInfo(mHandle, bar, &info);
    int delta = info.nMax - info.nPos;
    if (delta > 0)
    {
        delta = std::min(delta, mScrollLoResDelta[bar]);
        info.nPos += delta;
        SetScrollInfo(mHandle, bar, &info, TRUE);
    }
    return delta;
}

int Window::OnScrollDecrementLoRes(int bar)
{
    SCROLLINFO& info = mScrollInfo[bar];
    info.fMask = SIF_POS;
    GetScrollInfo(mHandle, bar, &info);
    int delta = info.nPos - info.nMin;
    if (delta > 0)
    {
        delta = std::min(delta, mScrollLoResDelta[bar]);
        info.nPos -= delta;
        SetScrollInfo(mHandle, bar, &info, TRUE);
    }
    return delta;
}

int Window::OnScrollIncrementHiRes(int bar)
{
    SCROLLINFO& info = mScrollInfo[bar];
    info.fMask = SIF_POS;
    GetScrollInfo(mHandle, bar, &info);
    int delta = info.nMax - info.nPos;
    if (delta > 0)
    {
        delta = std::min(delta, mScrollHiResDelta[bar]);
        info.nPos += delta;
        SetScrollInfo(mHandle, bar, &info, TRUE);
    }
    return delta;
}

int Window::OnScrollDecrementHiRes(int bar)
{
    SCROLLINFO& info = mScrollInfo[bar];
    info.fMask = SIF_POS;
    GetScrollInfo(mHandle, bar, &info);
    int delta = info.nPos - info.nMin;
    if (delta > 0)
    {
        delta = std::min(delta, mScrollHiResDelta[bar]);
        info.nPos -= delta;
        SetScrollInfo(mHandle, bar, &info, TRUE);
    }
    return delta;
}

int Window::OnScrollTracking(int bar)
{
    SCROLLINFO& info = mScrollInfo[bar];
    info.fMask = SIF_TRACKPOS;
    GetScrollInfo(mHandle, bar, &info);
    return info.nTrackPos;
}

int Window::OnScrollEndTracking(int bar)
{
    SCROLLINFO& info = mScrollInfo[bar];
    info.fMask = SIF_TRACKPOS;
    GetScrollInfo(mHandle, bar, &info);
    info.fMask = SIF_POS;
    info.nPos = info.nTrackPos;
    SetScrollInfo(mHandle, bar, &info, TRUE);
    return info.nTrackPos;
}

void Window::OnClose()
{
    PostQuitMessage(0);
}

