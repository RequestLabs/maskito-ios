// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
using namespace gte;

#define USE_ORIENTED_BOX

class IntersectBoxConeWindow : public Window3
{
public:
    IntersectBoxConeWindow(Parameters& parameters);

    virtual void OnIdle();

    virtual bool OnCharPress(unsigned char key, int x, int y);

private:
    void CreateScene();
    void InitializeCamera();
    void Translate(int direction, float delta);
    void Rotate(int direction, float delta);
    void TestIntersection();

    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<Visual> mConeMesh, mDiskMesh, mBoxMesh;
    std::shared_ptr<ConstantColorEffect> mRedEffect, mBlueEffect;
    Cone<3, float> mCone;
#if defined(USE_ORIENTED_BOX)
    TIOrientedBox3Cone3<float> mQuery;
    OrientedBox<3, float> mBox;
#else
    TIAlignedBox3Cone3<float> mQuery;
    AlignedBox<3, float> mBox;
#endif
};
