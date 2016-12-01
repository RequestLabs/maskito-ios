// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
#include "BlendedTerrainEffect.h"
using namespace gte;

class BlendedTerrainWindow : public Window3
{
public:
    BlendedTerrainWindow(Parameters& parameters);

    virtual void OnIdle();
    virtual bool OnCharPress(unsigned char key, int x, int y);

private:
    bool SetEnvironment();
    bool CreateTerrain();
    void CreateSkyDome();
    void InitializeCamera();
    void Update();

    std::shared_ptr<Visual> mTerrain;
    std::shared_ptr<Visual> mSkyDome;
    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<BlendedTerrainEffect> mTerrainEffect;
    std::shared_ptr<Texture2Effect> mSkyDomeEffect;
    float mFlowDelta, mPowerDelta, mZAngle, mZDeltaAngle;
};
