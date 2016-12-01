// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "VertexColoringWindow.h"


VertexColoringWindow::VertexColoringWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    CreateScene();
    InitializeCamera();

#if defined(SAVE_RENDERING_TO_DISK)
    mTarget = std::make_shared<DrawTarget>(1, DF_R8G8B8A8_UNORM, mXSize,
        mYSize);
    mTarget->GetRTTexture(0)->SetCopyType(Resource::COPY_STAGING_TO_CPU);
#endif
}

void VertexColoringWindow::OnIdle()
{
    mTimer.Measure();

    mCameraRig.Move();

    mEngine->ClearBuffers();
    mEngine->Draw(mTriangle);
    mEngine->DisplayColorBuffer(0);

#if defined(SAVE_RENDERING_TO_DISK)
    mEngine->Enable(mTarget);
    mEngine->ClearBuffers();
    mEngine->Draw(mTriangle);
    mEngine->Disable(mTarget);
    mEngine->CopyGpuToCpu(mTarget->GetRTTexture(0));
    WICFileIO::SaveToPNG("VertexColoring.png",
        mTarget->GetRTTexture(0).get());
#endif

    mTimer.UpdateFrameCount();
}

void VertexColoringWindow::CreateScene()
{
    // Create a vertex buffer for a single triangle.
    struct Vertex
    {
        Vector3<float> position;
        Vector4<float> color;
    };
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);
    std::shared_ptr<VertexBuffer> vbuffer(new VertexBuffer(vformat, 3));
    Vertex* vertex = vbuffer->Get<Vertex>();
    vertex[0].position = { 0.0f, 0.0f, 0.0f };
    vertex[0].color = { 1.0f, 0.0f, 0.0f, 1.0f };
    vertex[1].position = { 1.0f, 0.0f, 0.0f };
    vertex[1].color = { 0.0f, 1.0f, 0.0f, 1.0f };
    vertex[2].position = { 0.0f, 1.0f, 0.0f };
    vertex[2].color = { 0.0f, 0.0f, 1.0f, 1.0f };

    // Create an indexless buffer for a triangle mesh with one triangle.
    std::shared_ptr<IndexBuffer> ibuffer(new IndexBuffer(IP_TRIMESH, 1));

    // Create an effect for the vertex and pixel shaders.
    std::shared_ptr<VertexColorEffect> effect =
        std::make_shared<VertexColorEffect>(mProgramFactory);

    // Create the geometric object for drawing.  Translate it so that its
    // center of mass is at the origin.  This supports virtual trackball
    // motion about the object "center".
    mTriangle = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    float const negOneThird = -1.0f / 3.0f;
    mTriangle->localTransform.SetTranslation(negOneThird, negOneThird, 0.0f);

    // Enable automatic updates of pvw-matrices and w-matrices.
    mCameraRig.Subscribe(mTriangle->worldTransform,
        effect->GetPVWMatrixConstant());

    mTrackball.Attach(mTriangle);
    mTrackball.Update();
}

void VertexColoringWindow::InitializeCamera()
{
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 0.1f, 100.0f);
    Vector4<float> camPosition{ 0.0f, 0.0f, 1.25f, 1.0f };
    Vector4<float> camDVector{ 0.0f, 0.0f, -1.0f, 0.0f };
    Vector4<float> camUVector{ 0.0f, 1.0f, 0.0f, 0.0f };
    Vector4<float> camRVector = Cross(camDVector, camUVector);
    mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

    mTranslationSpeed = 0.001f;
    mRotationSpeed = 0.001f;
    mTranslationSpeedFactor = 2.0f;
    mRotationSpeedFactor = 2.0f;
    mCameraRig.SetTranslationSpeed(mTranslationSpeed);
    mCameraRig.SetRotationSpeed(mRotationSpeed);
    mCameraRig.ComputeWorldAxes();
    mCameraRig.UpdatePVWMatrices();
}

