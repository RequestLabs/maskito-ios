// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
using namespace gte;

class BlownGlassWindow : public Window3
{
public:
    BlownGlassWindow(Parameters& parameters);

    virtual void OnIdle();

private:
    bool SetEnvironment();
    bool CreateScene();
    void InitializeCamera();

    enum { GRID_SIZE = 128 };
    std::shared_ptr<Texture3Effect> mDrawEffect;
    std::shared_ptr<Visual> mMesh;
    std::unique_ptr<Fluid3> mFluid;
};
