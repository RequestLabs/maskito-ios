// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
using namespace gte;

class PlaneMeshIntersectionWindow : public Window3
{
public:
    PlaneMeshIntersectionWindow(Parameters& parameters);

    virtual void OnIdle();

private:
    bool SetEnvironment();
    bool CreateScene();
    void InitializeCamera();
    void UpdateMatrices();

    struct Vertex
    {
        Vector3<float> position, color;
    };

    struct PMIParameters
    {
        Matrix4x4<float> pvMatrix;
        Matrix4x4<float> wMatrix;
        Vector4<float> planeVector0;
        Vector4<float> planeVector1;
    };

    std::shared_ptr<ConstantBuffer> mPMIParameters;
    std::shared_ptr<Visual> mMesh;

    std::shared_ptr<DrawTarget> mPSTarget;
    std::shared_ptr<TextureRT> mPSColor;
    std::shared_ptr<TextureRT> mPSPlaneConstant;
    std::shared_ptr<Texture2> mScreen;
    std::shared_ptr<OverlayEffect> mOverlay;
    std::shared_ptr<ComputeProgram> mDrawIntersections;
};
