// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "IntersectBoxConeWindow.h"
#include <iostream>


IntersectBoxConeWindow::IntersectBoxConeWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cullMode = RasterizerState::CULL_NONE;
    mEngine->SetRasterizerState(mNoCullState);

    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cullMode = RasterizerState::CULL_NONE;
    mNoCullWireState->fillMode = RasterizerState::FILL_WIREFRAME;

    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::BM_SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::BM_INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::BM_SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::BM_INV_SRC_ALPHA;
    mEngine->SetBlendState(mBlendState);

    CreateScene();
    InitializeCamera();
    TestIntersection();
    mCameraRig.UpdatePVWMatrices();
}

void IntersectBoxConeWindow::OnIdle()
{
    mTimer.Measure();

    mCameraRig.Move();

    mEngine->ClearBuffers();
    mEngine->Draw(mConeMesh);
    mEngine->Draw(mDiskMesh);
    mEngine->Draw(mBoxMesh);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool IntersectBoxConeWindow::OnCharPress(unsigned char key, int x, int y)
{
    float const delta = 0.1f;
    Quaternion<float> q, incr;

    switch (key)
    {
    case 'w':
    case 'W':
        if (mEngine->GetRasterizerState() == mNoCullState)
        {
            mEngine->SetRasterizerState(mNoCullWireState);
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullState);
        }
        return true;

    case ' ':
        TestIntersection();
        return true;

    case 'x':  // decrement x-center of box
        Translate(0, -delta);
        return true;

    case 'X':  // increment x-center of box
        Translate(0, +delta);
        return true;

    case 'y':  // decrement y-center of box
        Translate(1, -delta);
        return true;

    case 'Y':  // increment y-center of box
        Translate(1, +delta);
        return true;

    case 'z':  // decrement z-center of box
        Translate(2, -delta);
        return true;

    case 'Z':  // increment z-center of box
        Translate(2, +delta);
        return true;

    case 'p':  // rotate about axis[0]
        Rotate(0, -delta);
        return true;

    case 'P':  // rotate about axis[0]
        Rotate(0, +delta);
        return true;

    case 'r':  // rotate about axis[1]
        Rotate(1, -delta);
        return true;

    case 'R':  // rotate about axis[1]
        Rotate(1, +delta);
        return true;

    case 'h':  // rotate about axis[2]
        Rotate(2, -delta);
        return true;

    case 'H':  // rotate about axis[2]
        Rotate(2, +delta);
        return true;
    }

    return Window::OnCharPress(key, x, y);
}

void IntersectBoxConeWindow::CreateScene()
{
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    mCone.ray.origin = { 0.0f, 0.0f, 0.0f };
    mCone.ray.direction = { 0.0f, 0.0f, 1.0f };
    //mCone.SetAngle((float)GTE_C_QUARTER_PI);
    mCone.SetAngle(0.25f);
    mCone.height = 16.0f;

    float const radius = mCone.height * tan(mCone.angle);
    mConeMesh = mf.CreateDisk(16, 16, radius);
    std::shared_ptr<VertexBuffer> vbuffer = mConeMesh->GetVertexBuffer();
    unsigned int numVertices = vbuffer->GetNumElements();
    Vector3<float>* vertex = vbuffer->Get<Vector3<float>>();
    float cotAngle = mCone.cosAngle / mCone.sinAngle;
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        Vector3<float>& P = vertex[i];
        P[2] = cotAngle * sqrt(P[0] * P[0] + P[1] * P[1]);
    }

    std::shared_ptr<ConstantColorEffect> effect =
        std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>({ 0.0f, 0.5f, 0.0f, 0.5f }));
    mConeMesh->SetEffect(effect);
    mCameraRig.Subscribe(mConeMesh->worldTransform,
        effect->GetPVWMatrixConstant());

    mDiskMesh = mf.CreateDisk(16, 16, radius);
    mDiskMesh->localTransform.SetTranslation(0.0f, 0.0f, mCone.height);
    mDiskMesh->Update();
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>({ 0.0f, 0.5f, 0.0f, 0.5f }));
    mDiskMesh->SetEffect(effect);
    mCameraRig.Subscribe(mDiskMesh->worldTransform,
        effect->GetPVWMatrixConstant());

    mRedEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>({ 0.5f, 0.0f, 0.0f, 0.5f }));

    mBlueEffect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>({ 0.0f, 0.0f, 0.5f, 0.5f }));

    Vector3<float> extent{ 1.0f, 2.0f, 3.0f };
#if defined(USE_ORIENTED_BOX)
    mBox.center = { 0.0f, 0.0f, 0.0f };
    mBox.axis[0] = { 1.0f, 0.0f, 0.0f };
    mBox.axis[1] = { 0.0f, 1.0f, 0.0f };
    mBox.axis[2] = { 0.0f, 0.0f, 1.0f };
    mBox.extent = extent;
#else
    mBox.min = -extent;
    mBox.max = +extent;
#endif

    mBoxMesh = mf.CreateBox(extent[0], extent[1], extent[2]);
    mBoxMesh->SetEffect(mBlueEffect);
    mCameraRig.Subscribe(mBoxMesh->worldTransform,
        mBlueEffect->GetPVWMatrixConstant());

    mTrackball.Attach(mConeMesh);
    mTrackball.Attach(mDiskMesh);
    mTrackball.Attach(mBoxMesh);
    mTrackball.Update();
}

void IntersectBoxConeWindow::InitializeCamera()
{
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 1.0f, 5000.0f);
    Vector4<float> camPosition{ 0.0f, 0.0f, -24.0f, 1.0f };
    Vector4<float> camDVector{ 0.0f, 0.0f, 1.0f, 0.0f };
    Vector4<float> camUVector{ 0.0f, 1.0f, 0.0f, 0.0f };
    Vector4<float> camRVector = Cross(camDVector, camUVector);
    mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

    mTranslationSpeed = 0.1f;
    mRotationSpeed = 0.01f;
    mTranslationSpeedFactor = 2.0f;
    mRotationSpeedFactor = 2.0f;
    mCameraRig.SetTranslationSpeed(mTranslationSpeed);
    mCameraRig.SetRotationSpeed(mRotationSpeed);
    mCameraRig.ComputeWorldAxes();
    mCameraRig.UpdatePVWMatrices();
}

void IntersectBoxConeWindow::Translate(int direction, float delta)
{
#if defined(USE_ORIENTED_BOX)
    mBox.center[direction] += delta;
    mBoxMesh->localTransform.SetTranslation(mBox.center);
#else
    mBox.min[direction] += delta;
    mBox.max[direction] += delta;
    mBoxMesh->localTransform.SetTranslation(0.5f*(mBox.min + mBox.max));
#endif
    mBoxMesh->Update();
    TestIntersection();
    mCameraRig.UpdatePVWMatrices();
}

void IntersectBoxConeWindow::Rotate(int direction, float delta)
{
#if defined(USE_ORIENTED_BOX)
    Quaternion<float> incr = Rotation<3, float>(
        AxisAngle<3, float>(mBox.axis[direction], delta));
    for (int i = 0; i < 3; ++i)
    {
        if (i != direction)
        {
            mBox.axis[i] = HProject(
                gte::Rotate(incr, HLift(mBox.axis[i], 0.0f)));
        }
    }
    Quaternion<float> q;
    mBoxMesh->localTransform.GetRotation(q);
    mBoxMesh->localTransform.SetRotation(incr * q);
    mBoxMesh->Update();
    TestIntersection();
    mCameraRig.UpdatePVWMatrices();
#else
    (void)direction;
    (void)delta;
#endif
}

void IntersectBoxConeWindow::TestIntersection()
{
    mCameraRig.Unsubscribe(mBoxMesh->worldTransform);

    if (mQuery(mBox, mCone).intersect)
    {
        mBoxMesh->SetEffect(mRedEffect);
        mCameraRig.Subscribe(mBoxMesh->worldTransform,
            mRedEffect->GetPVWMatrixConstant());
    }
    else
    {
        mBoxMesh->SetEffect(mBlueEffect);
        mCameraRig.Subscribe(mBoxMesh->worldTransform,
            mBlueEffect->GetPVWMatrixConstant());
    }
}

