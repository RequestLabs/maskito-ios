// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
using namespace gte;

class CameraAndLightNodesWindow : public Window3
{
public:
    CameraAndLightNodesWindow(Parameters& parameters);

    virtual void OnIdle();
    virtual bool OnCharPress(unsigned char key, int x, int y);
    virtual bool OnKeyDown(int key, int x, int y);
    virtual bool OnKeyUp(int key, int x, int y);

private:
    bool SetEnvironment();
    void InitializeCameraNode();
    void CreateScene();
    std::shared_ptr<Visual> CreateGround();
    std::shared_ptr<Node> CreateLightFixture(int i);
    std::shared_ptr<Visual> CreateLightTarget();

    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<DepthStencilState> mNoDepthStencilState;
    std::shared_ptr<OverlayEffect> mOverlay;
    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mGround, mLightTarget[2];
    std::shared_ptr<PointLightEffect> mEffect[2];

    // Support for the camera node and the light nodes.
    class CameraNodeRig : public CameraRig
    {
    public:
        void SetCameraNode(std::shared_ptr<ViewVolumeNode> const& cameraNode);

    private:
        virtual void MoveForward();
        virtual void MoveBackward();
        virtual void TurnRight();
        virtual void TurnLeft();

        std::shared_ptr<ViewVolumeNode> mCameraNode;
    };

    std::shared_ptr<ViewVolumeNode> mCameraNode, mLightNode[2];
    Vector4<float> mCameraModelPosition;
    CameraNodeRig mCameraNodeRig;
};
