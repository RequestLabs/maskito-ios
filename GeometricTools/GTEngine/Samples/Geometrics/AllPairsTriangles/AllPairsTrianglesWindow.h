// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
#include "TriangleIntersection.h"
using namespace gte;

//#define USE_CPU_FIND_INTERSECTIONS

class AllPairsTrianglesWindow : public Window3
{
public:
    AllPairsTrianglesWindow(Parameters& parameters);

    virtual void OnIdle();

    virtual bool OnCharPress(unsigned char key, int x, int y);

private:
    bool SetEnvironment();
    void InitializeCamera();

    bool CreateCylinder(unsigned int numAxisSamples,
        unsigned int numRadialSamples, float radius, float height);

    bool CreateTorus(unsigned int numCircleSamples,
        unsigned int numRadialSamples, float outerRadius, float innerRadius);

#if !defined(USE_CPU_FIND_INTERSECTIONS)
    bool CreateShaders();
#endif

    void UpdateTransforms();
    void FindIntersections();

    Environment mEnvironment;
    Vector4<float> mTextColor;
    std::shared_ptr<RasterizerState> mWireState;

    struct Vertex
    {
        Vector3<float> position;
        unsigned int colorIndex;
    };

    unsigned int mNumCylinderTriangles, mNumTorusTriangles;
    std::shared_ptr<Visual> mCylinder, mTorus;
    std::shared_ptr<VisualEffect> mCylinderEffect, mTorusEffect;
    std::shared_ptr<ConstantBuffer> mCylinderPVWMatrix, mTorusPVWMatrix;

#if !defined(USE_CPU_FIND_INTERSECTIONS)
    unsigned int mNumXGroups, mNumYGroups;

    std::shared_ptr<StructuredBuffer> mColor0Buffer, mColor1Buffer;
    std::shared_ptr<ComputeProgram> mInitializeColor;

    struct TIParameters
    {
        Matrix4x4<float> wMatrix0, wMatrix1;
        unsigned int numTriangles0, numTriangles1;
    };
    std::shared_ptr<ConstantBuffer> mTIParameters;
    std::shared_ptr<StructuredBuffer> mVertices0, mVertices1;
    std::shared_ptr<ComputeProgram> mTriangleIntersection;

    std::shared_ptr<VisualEffect> mCylinderIDEffect, mTorusIDEffect;
    std::shared_ptr<Visual> mCylinderID, mTorusID;
#endif
};
