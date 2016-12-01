// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "WireMeshWindow.h"


WireMeshWindow::WireMeshWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment() || !CreateScene())
    {
        parameters.created = false;
        return;
    }

    InitializeCamera();
}

void WireMeshWindow::OnIdle()
{
    mTimer.Measure();
    mCameraRig.Move();

    mEngine->ClearBuffers();
    mEngine->Draw(mMesh);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0 }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool WireMeshWindow::SetEnvironment()
{
    std::string path = mEnvironment.GetVariable("GTE_PATH");
    if (path == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Graphics/WireMesh/Shaders/");

    if (mEnvironment.GetPath("WireMesh.hlsl") == "")
    {
        LogError("Cannot find file WireMesh.hlsl.");
        return false;
    }

    return true;
}

bool WireMeshWindow::CreateScene()
{
    std::string path = mEnvironment.GetPath("WireMesh.hlsl");
    std::shared_ptr<VisualProgram> program =
        mProgramFactory.CreateFromFiles(path, path, path);
    if (!program)
    {
        return false;
    }

    std::shared_ptr<ConstantBuffer> parameters(
        new ConstantBuffer(3 * sizeof(Vector4<float>), false));
    Vector4<float>* data = parameters->Get<Vector4<float>>();
    data[0] = { 0.0f, 0.0f, 1.0f, 1.0f };  // mesh color
    data[1] = { 0.0f, 0.0f, 0.0f, 1.0f };  // edge color
    data[2] = { (float)mXSize, (float)mYSize, 0.0f, 0.0f };
    program->GetVShader()->Set("WireParameters", parameters);
    program->GetPShader()->Set("WireParameters", parameters);
    program->GetGShader()->Set("WireParameters", parameters);

    std::shared_ptr<ConstantBuffer> cbuffer(
        new ConstantBuffer(sizeof(Matrix4x4<float>), true));
    program->GetVShader()->Set("PVWMatrix", cbuffer);

    std::shared_ptr<VisualEffect> effect =
        std::make_shared<VisualEffect>(program);

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mMesh = mf.CreateSphere(16, 16, 1.0f);
    mMesh->SetEffect(effect);

    mCameraRig.Subscribe(mMesh->worldTransform, cbuffer);

    mTrackball.Attach(mMesh);
    mTrackball.Update();
    return true;
}

void WireMeshWindow::InitializeCamera()
{
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 0.1f, 100.0f);
    Vector4<float> camPosition{ 0.0f, 0.0f, -2.5f, 1.0f };
    Vector4<float> camDVector{ 0.0f, 0.0f, 1.0f, 0.0f };
    Vector4<float> camUVector{ 0.0f, 1.0f, 0.0f, 0.0f };
    Vector4<float> camRVector = Cross(camDVector, camUVector);
    mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

    mTranslationSpeed = 0.01f;
    mRotationSpeed = 0.001f;
    mTranslationSpeedFactor = 2.0f;
    mRotationSpeedFactor = 2.0f;
    mCameraRig.SetTranslationSpeed(mTranslationSpeed);
    mCameraRig.SetRotationSpeed(mRotationSpeed);
    mCameraRig.ComputeWorldAxes();
    mCameraRig.UpdatePVWMatrices();
}

