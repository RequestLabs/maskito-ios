// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Applications/GteWindow3.h>
using namespace gte;


Window3::Window3(Parameters& parameters)
    :
    Window(parameters),
    mUpdater([this](std::shared_ptr<Buffer> const& buffer){ mEngine->Update(buffer); }),
    mCamera(new Camera()),
    mCameraRig(mCamera, 0.0f, 0.0f, mUpdater),
    mTrackball(mXSize, mYSize, mCamera),
    mTranslationSpeed(0.0f),
    mRotationSpeed(0.0f),
    mTranslationSpeedFactor(1.0f),
    mRotationSpeedFactor(1.0f)
{
    mCameraRig.RegisterMoveForward(VK_UP);
    mCameraRig.RegisterMoveBackward(VK_DOWN);
    mCameraRig.RegisterMoveUp(VK_HOME);
    mCameraRig.RegisterMoveDown(VK_END);
    mCameraRig.RegisterMoveRight(VK_INSERT);
    mCameraRig.RegisterMoveLeft(VK_DELETE);
    mCameraRig.RegisterTurnRight(VK_RIGHT);
    mCameraRig.RegisterTurnLeft(VK_LEFT);
    mCameraRig.RegisterLookUp(VK_PRIOR);
    mCameraRig.RegisterLookDown(VK_NEXT);
}

bool Window3::OnResize(int xSize, int ySize)
{
    if (Window::OnResize(xSize, ySize))
    {
        float upFovDegrees, aspectRatio, dMin, dMax;
        mCamera->GetFrustum(upFovDegrees, aspectRatio, dMin, dMax);
        mCamera->SetFrustum(upFovDegrees, GetAspectRatio(), dMin, dMax);
        mCameraRig.UpdatePVWMatrices();
    }
    return false;
}

bool Window3::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 't':  // Slower camera translation.
        mTranslationSpeed /= mTranslationSpeedFactor;
        mCameraRig.SetTranslationSpeed(mTranslationSpeed);
        return true;

    case 'T':  // Faster camera translation.
        mTranslationSpeed *= mTranslationSpeedFactor;
        mCameraRig.SetTranslationSpeed(mTranslationSpeed);
        return true;

    case 'r':  // Slower camera rotation.
        mRotationSpeed /= mRotationSpeedFactor;
        mCameraRig.SetRotationSpeed(mRotationSpeed);
        return true;

    case 'R':  // Faster camera rotation.
        mRotationSpeed *= mRotationSpeedFactor;
        mCameraRig.SetRotationSpeed(mRotationSpeed);
        return true;
    }

    return Window::OnCharPress(key, x, y);
}

bool Window3::OnKeyDown(int key, int, int)
{
    return mCameraRig.PushMotion(key);
}

bool Window3::OnKeyUp(int key, int, int)
{
    return mCameraRig.PopMotion(key);
}

bool Window3::OnMouseClick(MouseButton button, MouseState state, int x, int y,
    unsigned int)
{
    if (button == MOUSE_LEFT)
    {
        if (state == MOUSE_DOWN)
        {
            mTrackball.SetActive(true);
            mTrackball.SetInitialPoint(x, mYSize - 1 - y);
        }
        else
        {
            mTrackball.SetActive(false);
        }

        return true;
    }

    return false;
}

bool Window3::OnMouseMotion(MouseButton button, int x, int y, unsigned int)
{
    if (button == MOUSE_LEFT && mTrackball.GetActive())
    {
        mTrackball.SetFinalPoint(x, mYSize - 1 - y);
        mCameraRig.UpdatePVWMatrices();
        return true;
    }

    return false;
}

