// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
using namespace gte;

class BspNodesWindow : public Window3
{
public:
    BspNodesWindow(Parameters& parameters);

    virtual void OnIdle();
    virtual bool OnMouseMotion(MouseButton button, int x, int y, unsigned int modifiers);

private:
    bool SetEnvironment();
    void CreateScene();
    void InitializeCamera();
    void DoCullSort();

    std::shared_ptr<BspNode> CreateNode(int i, Vector2<float> const& v0,
        Vector2<float> const& v1, Vector4<float> const& color);

    Culler mCuller;
    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mGround;
    std::shared_ptr<RasterizerState> mNoCullWireState;
    std::shared_ptr<BspNode> mBSPNode[4];
    std::shared_ptr<Visual> mRectangle[4];
    std::shared_ptr<VertexColorEffect> mVCEffect[4];
    std::shared_ptr<Visual> mTorus, mSphere, mTetrahedron, mCube, mOctahedron;
    std::set<Visual*> mVisualOpaque;
    std::vector<Visual*> mVisibleOpaque;
    std::vector<Visual*> mVisibleNoCullWire;
};
