// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Applications/GteWindow.h>
#include <Graphics/GteCameraRig.h>
#include <Graphics/GteTrackball.h>

namespace gte
{

class GTE_IMPEXP Window3 : public Window
{
protected:
    // Abstract base class.
    Window3(Parameters& parameters);

public:
    // The camera frustum is modified.  Any subscribers to the pvw-matrix
    // update system of the camera rig must be updated.  No call is made to
    // OnDisplay() or OnIdle().  The base class is unaware of which display
    // method you use, so to have a visual update you must override OnResize
    //    bool MyApplication::OnResize(int xSize, int ySize)
    //    {
    //        if (Window3::OnResize(xSize, ySize))
    //         {
    //             OnIdle();  // or OnDisplay() or MyOwnDrawFunction() ...
    //         }
    //    }
    virtual bool OnResize(int xSize, int ySize);

    // The key 't' decreases the translation speed and the 'T' key increases
    // the translation speed.  The 'r' key decreases the rotation speed and
    // the 'R' key increases the rotation speed.
    virtual bool OnCharPress(unsigned char key, int x, int y);

    // The appropriate camera rig motion is selected when 'key' is mapped
    // to a camera motion.
    virtual bool OnKeyDown(int key, int x, int y);
    virtual bool OnKeyUp(int key, int x, int y);

    // Control the rotation of the trackball.
    virtual bool OnMouseClick(MouseButton button, MouseState state,
        int x, int y, unsigned int modifiers);
    virtual bool OnMouseMotion(MouseButton button, int x, int y,
        unsigned int modifiers);

protected:
    BufferUpdater mUpdater;
    std::shared_ptr<Camera> mCamera;
    CameraRig mCameraRig;
    Trackball mTrackball;
    float mTranslationSpeed, mRotationSpeed;
    float mTranslationSpeedFactor, mRotationSpeedFactor;
};

}
