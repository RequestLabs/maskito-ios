// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "MinimumVolumeSphere3DWindow.h"


MinimumVolumeSphere3DWindow::MinimumVolumeSphere3DWindow(Parameters& parameters)
    :
    Window3(parameters),
    mNumActive(2),
    mVertices(NUM_POINTS)
{
    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cullMode = RasterizerState::CULL_NONE;
    mNoCullWireState->fillMode = RasterizerState::FILL_WIREFRAME;
    mEngine->SetRasterizerState(mNoCullWireState);

    CreateScene();
    InitializeCamera();

    mMVS3(mNumActive, &mVertices[0], mMinimalSphere);
    UpdateScene();
}

void MinimumVolumeSphere3DWindow::OnIdle()
{
    mTimer.Measure();

    mCameraRig.Move();

    mEngine->ClearBuffers();
    for (int i = 0; i < mNumActive; ++i)
    {
        mEngine->Draw(mPoints[i]);
    }
    mEngine->Draw(mSegments);
    mEngine->Draw(mSphere);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool MinimumVolumeSphere3DWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'n':
    case 'N':
        if (mNumActive < NUM_POINTS)
        {
            mMVS3(++mNumActive, &mVertices[0], mMinimalSphere);
            UpdateScene();
        }
        return true;
    }

    return Window::OnCharPress(key, x, y);
}

void MinimumVolumeSphere3DWindow::CreateScene()
{
    // Create the points.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(-1.0f, 1.0f);
    for (auto& v : mVertices)
    {
        v = { rnd(mte), rnd(mte), rnd(mte) };
    }

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    std::shared_ptr<ConstantColorEffect> effect;
    for (int i = 0; i < NUM_POINTS; ++i)
    {
        mPoints[i] = mf.CreateSphere(6, 6, 0.01f);
        effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
            Vector4<float>({ 0.5f, 0.5f, 0.5f, 1.0f }));
        mPoints[i]->SetEffect(effect);
        mCameraRig.Subscribe(mPoints[i]->worldTransform,
            effect->GetPVWMatrixConstant());

        std::shared_ptr<VertexBuffer> vbuffer = mPoints[i]->GetVertexBuffer();
        Vector3<float>* vertex = vbuffer->Get<Vector3<float>>();
        Vector3<float> offset = mVertices[i];
        for (unsigned int j = 0; j < vbuffer->GetNumElements(); ++j)
        {
            vertex[j] += offset;
        }
    }

    // Create the segments.
    std::shared_ptr<VertexBuffer> vbuffer(new VertexBuffer(vformat, 12));
    vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    std::shared_ptr<IndexBuffer> ibuffer(new IndexBuffer(
        IP_POLYSEGMENT_DISJOINT, 6));
    effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>({ 0.5f, 0.0f, 0.0f, 1.0f }));
    mSegments = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mCameraRig.Subscribe(mSegments->worldTransform,
        effect->GetPVWMatrixConstant());
    mSegments->Update();

    // Create the sphere.
    mSphere = mf.CreateSphere(16, 16, 1.0f);

    effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>({ 0.0f, 0.0f, 0.5f, 1.0f }));

    mSphere->SetEffect(effect);
    mCameraRig.Subscribe(mSphere->worldTransform,
        effect->GetPVWMatrixConstant());
}

void MinimumVolumeSphere3DWindow::InitializeCamera()
{
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 0.1f, 100.0f);
    Vector4<float> camPosition{ 0.0f, 0.0f, -4.0f, 1.0f };
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

void MinimumVolumeSphere3DWindow::UpdateScene()
{
    // Update the segments.
    std::shared_ptr<VertexBuffer> vbuffer = mSegments->GetVertexBuffer();
    Vector3<float>* vertex = vbuffer->Get<Vector3<float>>();
    std::shared_ptr<IndexBuffer> ibuffer = mSegments->GetIndexBuffer();

    int numSupport = mMVS3.GetNumSupport();
    std::array<int, 4> support = mMVS3.GetSupport();

    if (numSupport >= 2)
    {
        vertex[0] = mVertices[support[0]];
        vertex[1] = mVertices[support[1]];
        vbuffer->SetNumActiveElements(2);
    }

    if (numSupport >= 3)
    {
        vertex[2] = mVertices[support[1]];
        vertex[3] = mVertices[support[2]];
        vertex[4] = mVertices[support[2]];
        vertex[5] = mVertices[support[0]];
        vbuffer->SetNumActiveElements(6);
    }

    if (numSupport == 4)
    {
        vertex[ 6] = mVertices[support[3]];
        vertex[ 7] = mVertices[support[0]];
        vertex[ 8] = mVertices[support[3]];
        vertex[ 9] = mVertices[support[1]];
        vertex[10] = mVertices[support[3]];
        vertex[11] = mVertices[support[2]];
        vbuffer->SetNumActiveElements(12);
    }

    mEngine->Update(vbuffer);

    // Update the sphere.
    mSphere->localTransform.SetTranslation(mMinimalSphere.center);
    mSphere->localTransform.SetUniformScale(mMinimalSphere.radius);
    mSphere->Update();

    mCameraRig.UpdatePVWMatrices();
}

