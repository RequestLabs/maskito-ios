// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
using namespace gte;

class BumpMapsWindow : public Window3
{
public:
    BumpMapsWindow(Parameters& parameters);

    virtual void OnIdle();
    virtual bool OnCharPress(unsigned char key, int x, int y);
    virtual bool OnMouseMotion(MouseButton button, int x, int y, unsigned int modifiers);

private:
    bool SetEnvironment();
    bool CreateScene();
    bool CreateTorus();
    void InitializeCamera();
    void UpdateBumpMap();

    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mTorus;
    Vector4<float> mLightDirection;
    bool mUseBumpMap;
};
