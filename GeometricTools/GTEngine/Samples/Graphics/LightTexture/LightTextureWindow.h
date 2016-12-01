// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
using namespace gte;

class LightTextureWindow : public Window3
{
public:
    LightTextureWindow(Parameters& parameters);

    virtual void OnIdle();
    virtual bool OnCharPress(unsigned char key, int x, int y);

private:
    bool SetEnvironment();
    void InitializeCamera();
    void CreateScene();
    void UpdateConstants();

    std::shared_ptr<DirectionalLightTextureEffect> mDLTEffect;
    std::shared_ptr<PointLightTextureEffect> mPLTEffect;
    Vector4<float> mLightWorldPosition;
    Vector4<float> mLightWorldDirection;
    std::shared_ptr<Visual> mTerrain;
    bool mUseDirectional;
};
