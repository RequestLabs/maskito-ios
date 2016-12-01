// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
using namespace gte;

class MinimumVolumeSphere3DWindow : public Window3
{
public:
    MinimumVolumeSphere3DWindow(Parameters& parameters);

    virtual void OnIdle();

    virtual bool OnCharPress(unsigned char key, int x, int y);

private:
    void CreateScene();
    void InitializeCamera();
    void UpdateScene();

    enum { NUM_POINTS = 256 };
    std::array<std::shared_ptr<Visual>, NUM_POINTS> mPoints;
    std::shared_ptr<Visual> mSegments, mSphere;
    std::shared_ptr<RasterizerState> mNoCullWireState;

    int mNumActive;
    std::vector<Vector3<float>> mVertices;
    Sphere3<float> mMinimalSphere;
    MinimumVolumeSphere3<float, BSRational<UIntegerAP32>> mMVS3;
};
