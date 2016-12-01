// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
using namespace gte;

class Delaunay3DWindow : public Window3
{
public:
    Delaunay3DWindow(Parameters& parameters);

    virtual void OnIdle();

    virtual bool OnCharPress(unsigned char key, int x, int y);

private:
    bool SetEnvironment();
    bool CreateScene();
    void CreateSphere();
    void CreateTetra(int index);
    void InitializeCamera();
    void SetAllTetraWire();
    void SetTetraSolid(int index, Vector4<float> const& color);
    void SetLastTetraSolid(Vector4<float> const& color,
        Vector4<float> const& oppositeColor);
    void DoSearch();

    Vector4<float> mLightGray;
    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<VertexColorEffect> mVCEffect;

    struct Vertex
    {
        Vector3<float> position;
        Vector4<float> color;
    };

    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mSphere;
    std::vector<std::shared_ptr<Visual>> mWireTetra;
    std::vector<std::shared_ptr<Visual>> mSolidTetra;

    std::vector<Vector3<float>> mVertices;
    std::mt19937 mRandomGenerator;
    std::uniform_real_distribution<float> mRandom[3];

    // The choice of 12 is empirical.  All the data sets tested in this
    // sample require at most 11 elements in the UIntegerFP32 array.
    Delaunay3<float, BSNumber<UIntegerFP32<12>>> mDelaunay;
    Delaunay3<float, BSNumber<UIntegerFP32<12>>>::SearchInfo mInfo;
};
