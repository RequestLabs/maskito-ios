// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Applications/GteEnvironment.h>
#include <Applications/GteOnIdleTimer.h>

#if defined(GTE_DEV_OPENGL)
#include <Graphics/GL4/GteGL4Engine.h>
#include <Graphics/GL4/GteGLSLProgramFactory.h>
#else
#include <Graphics/DX11/GteDX11Engine.h>
#include <Graphics/DX11/GteHLSLProgramFactory.h>
#endif

namespace gte
{

class GTE_IMPEXP Window
{
public:
    // All parameters for constructing windows are in this structure.  Derived
    // classes may define nested classes to derive from this.  The creation by
    // WindowSystem requires you to pass such a structure.
    struct GTE_IMPEXP Parameters
    {
        Parameters()
            :
            title(L""),
            xOrigin(0),
            yOrigin(0),
            xSize(0),
            ySize(0),
            allowResize(false),
            hscrollBar(false),
            vscrollBar(false),
            created(false),
            deviceCreationFlags(0),
            featureLevel(D3D_FEATURE_LEVEL_11_0),
            handle(nullptr),
            engine(nullptr)
        {
        }

        Parameters(std::wstring const& inTitle, int inXOrigin, int inYOrigin,
            int inXSize, int inYSize)
            :
            title(inTitle),
            xOrigin(inXOrigin),
            yOrigin(inYOrigin),
            xSize(inXSize),
            ySize(inYSize),
            allowResize(false),
            hscrollBar(false),
            vscrollBar(false),
            created(false),
            deviceCreationFlags(0),
            featureLevel(D3D_FEATURE_LEVEL_11_0),
            handle(nullptr),
            engine(nullptr)
        {
        }

        std::wstring title;
        int xOrigin, yOrigin, xSize, ySize;
        bool allowResize, hscrollBar, vscrollBar, created;
        UINT deviceCreationFlags;
        D3D_FEATURE_LEVEL featureLevel;
        HWND handle;
#if defined(GTE_DEV_OPENGL)
        GL4Engine* engine;
#else
        DX11Engine* engine;
#endif
    };

protected:
    // Abstract base class.  Only WindowSystem may create windows.
    Window(Parameters& parameters);
public:
    virtual ~Window();

    // Member access.
    inline HWND GetHandle() const;
    inline void SetTitle(std::wstring const& title);
    inline std::wstring GetTitle() const;
    inline int GetXOrigin() const;
    inline int GetYOrigin() const;
    inline int GetXSize() const;
    inline int GetYSize() const;
    inline bool IsMinimized() const;
    inline bool IsMaximized() const;
    inline float GetAspectRatio() const;

    // Display callbacks.
    virtual void OnMove(int x, int y);
    virtual bool OnResize(int xSize, int ySize);
    virtual void OnMinimize();
    virtual void OnMaximize();
    virtual void OnDisplay();
    virtual void OnIdle();

    // Keyboard callbacks.  OnCharPress allows you to distinguish between
    // upper-case and lower-case letters; OnKeyDown and OnKeyUp do not.
    // In OnCharPress, the ESC key terminates the application by calling
    // OnClose, which in turn posts a quit message.  Also, the space bar
    // (key ' ') resets the OnIdleTimer object.
    virtual bool OnCharPress(unsigned char key, int x, int y);
    virtual bool OnKeyDown(int key, int x, int y);
    virtual bool OnKeyUp(int key, int x, int y);

    // Mouse callbacks and state information.
    enum GTE_IMPEXP MouseButton
    {
        MOUSE_NONE,
        MOUSE_LEFT,
        MOUSE_MIDDLE,
        MOUSE_RIGHT
    };

    enum GTE_IMPEXP MouseState
    {
        MOUSE_UP,
        MOUSE_DOWN
    };

    virtual bool OnMouseClick(MouseButton button, MouseState state,
        int x, int y, unsigned int modifiers);

    virtual bool OnMouseMotion(MouseButton button, int x, int y,
        unsigned int modifiers);

    // The MSDN documentation for WM_MOUSEWHEEL states that the 'delta'
    // stored in the high-order word of wParam is the distance the wheel is
    // rotated, in multiples of WHEEL_DELTA.  This constant is 120.  Later in
    // the documentation, presumably something added later, there is mention
    // that finer-resolution wheels might report 'delta' smaller than 120.
    // The previous version of OnMouseWheel passed 'delta' but divided by
    // WHEEL_DELTA; for finer-resolution wheels, the input 'delta' to
    // OnMouseWheel was typically 0.  Now the input is simply the number
    // reported in wParam.
    virtual bool OnMouseWheel(int delta, int x, int y,
        unsigned int modifiers);

    void SetMousePosition(int x, int y);
    void GetMousePosition(int& x, int& y) const;

    // Scroll support.  The 'bar' value is 0 for horizontal scroll bars or 1
    // for vertical scroll bars.
    void SetScrollInterval(int bar, int minValue, int maxValue);
    void GetScrollInterval(int bar, int& minValue, int& maxValue) const;
    int SetScrollPosition(int bar, int value);
    int GetScrollPosition(int bar) const;

    // The return value of the increment/decrement functions is the delta of
    // the slider thumb.  If zero, the scroll state did not change.  For the
    // tracking functions, the return value is the current slider thumb
    // position.  A derived-class override must call the base-class function
    // first.
    virtual int OnScrollIncrementLoRes(int bar);
    virtual int OnScrollDecrementLoRes(int bar);
    virtual int OnScrollIncrementHiRes(int bar);
    virtual int OnScrollDecrementHiRes(int bar);
    virtual int OnScrollTracking(int bar);
    virtual int OnScrollEndTracking(int bar);

    // Actions to take before the window closes.
    virtual void OnClose();

protected:
    // Standard window information.
    HWND mHandle;
    std::wstring mTitle;
    int mXOrigin, mYOrigin, mXSize, mYSize;
    bool mAllowResize;
    bool mIsMinimized;
    bool mIsMaximized;

    // Scroll bar support.
    bool mHasScroll[2];
    mutable SCROLLINFO mScrollInfo[2];
    int mScrollLoResDelta[2];
    int mScrollHiResDelta[2];

    Environment mEnvironment;
    OnIdleTimer mTimer;

    // Graphics device and camera.
#if defined(GTE_DEV_OPENGL)
    GL4Engine* mEngine;
    GLSLProgramFactory mProgramFactory;
#else
    DX11Engine* mEngine;
    HLSLProgramFactory mProgramFactory;
#endif
};


inline HWND Window::GetHandle() const
{
    return mHandle;
}

inline std::wstring Window::GetTitle() const
{
    return mTitle;
}

inline int Window::GetXOrigin() const
{
    return mXOrigin;
}

inline int Window::GetYOrigin() const
{
    return mYOrigin;
}

inline int Window::GetXSize() const
{
    return mXSize;
}

inline int Window::GetYSize() const
{
    return mYSize;
}

inline bool Window::IsMinimized() const
{
    return mIsMinimized;
}

inline bool Window::IsMaximized() const
{
    return mIsMaximized;
}

inline float Window::GetAspectRatio() const
{
    return (float)mXSize / (float)mYSize;
}


}
