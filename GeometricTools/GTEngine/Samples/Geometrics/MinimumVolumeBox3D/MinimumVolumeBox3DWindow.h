// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
using namespace gte;

class MinimumVolumeBox3DWindow : public Window3
{
public:
    MinimumVolumeBox3DWindow(Parameters& parameters);

    virtual void OnIdle();

private:
    void CreateScene();
    void InitializeCamera();

    enum { NUM_POINTS = 2048 };
    std::vector<Vector3<float>> mVertices;
    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mPoints;
    std::shared_ptr<Visual> mPolytope;
    std::shared_ptr<Visual> mBoxMesh;
    std::shared_ptr<RasterizerState> mWireState;
};
