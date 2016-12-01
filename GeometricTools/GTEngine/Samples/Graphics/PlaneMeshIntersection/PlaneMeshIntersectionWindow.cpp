// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "PlaneMeshIntersectionWindow.h"


PlaneMeshIntersectionWindow::PlaneMeshIntersectionWindow(
    Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment() || !CreateScene())
    {
        parameters.created = false;
        return;
    }

    mPSTarget = std::make_shared<DrawTarget>(2, DF_R32G32B32A32_FLOAT,
        mXSize, mYSize, true, false, DF_D24_UNORM_S8_UINT, false);
    mPSColor = mPSTarget->GetRTTexture(0);
    mPSPlaneConstant = mPSTarget->GetRTTexture(1);

    mScreen = std::make_shared<Texture2>(DF_R32G32B32A32_FLOAT, mXSize,
        mYSize);
    mScreen->SetUsage(Resource::SHADER_OUTPUT);
    mScreen->SetCopyType(Resource::COPY_STAGING_TO_CPU);

    mOverlay = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, mXSize, mYSize, SamplerState::MIN_P_MAG_P_MIP_P,
        SamplerState::CLAMP, SamplerState::CLAMP, true);
    mOverlay->SetTexture(mScreen);

    mEngine->SetClearColor({ 1.0f, 1.0f, 1.0f,
        std::numeric_limits<float>::max() });

    std::shared_ptr<ComputeShader> cshader = mDrawIntersections->GetCShader();
    cshader->Set("color", mPSColor);
    cshader->Set("planeConstant", mPSPlaneConstant);
    cshader->Set("output", mScreen);

    InitializeCamera();
}

void PlaneMeshIntersectionWindow::OnIdle()
{
    mTimer.Measure();

    mCameraRig.Move();
    UpdateMatrices();

    mEngine->Enable(mPSTarget);
    mEngine->ClearBuffers();
    mEngine->Draw(mMesh);
    mEngine->Disable(mPSTarget);
    mEngine->Execute(mDrawIntersections->GetCShader(), mXSize / 8,
        mYSize / 8, 1);
    mEngine->Draw(mOverlay);
    mEngine->Draw(8, mYSize - 8, { 1.0f, 1.0f, 1.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool PlaneMeshIntersectionWindow::SetEnvironment()
{
    std::string path = mEnvironment.GetVariable("GTE_PATH");
    if (path == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return false;
    }

    mEnvironment.Insert(
        path + "/Samples/Graphics/PlaneMeshIntersection/Shaders/");

    if (mEnvironment.GetPath("PlaneMeshIntersection.hlsl") == "")
    {
        LogError("Cannot find file PlaneMeshIntersection.hlsl.");
        return false;
    }

    if (mEnvironment.GetPath("DrawIntersections.hlsl") == "")
    {
        LogError("Cannot find file DrawIntersections.hlsl.");
        return false;
    }

    return true;
}

bool PlaneMeshIntersectionWindow::CreateScene()
{
    std::string path = mEnvironment.GetPath("PlaneMeshIntersection.hlsl");
    std::shared_ptr<VisualProgram> program =
        mProgramFactory.CreateFromFiles(path, path, "");
    if (!program)
    {
        return false;
    }

    path = mEnvironment.GetPath("DrawIntersections.hlsl");
    mDrawIntersections = mProgramFactory.CreateFromFile(path);
    if (!mDrawIntersections)
    {
        return false;
    }

    float planeDelta = 0.125f;
    mPMIParameters = std::make_shared<ConstantBuffer>(sizeof(PMIParameters),
        true);
    PMIParameters& p = *mPMIParameters->Get<PMIParameters>();
    p.pvMatrix = mCamera->GetProjectionViewMatrix();
    p.wMatrix = Matrix4x4<float>::Identity();
    p.planeVector0 = Vector4<float>{ 1.0f, 0.0f, 0.0f, 0.0f } / planeDelta;
    p.planeVector1 = Vector4<float>{ 0.0f, 1.0f, 0.0f, 0.0f } / planeDelta;
    program->GetVShader()->Set("PMIParameters", mPMIParameters);

    std::shared_ptr<VisualEffect> effect =
        std::make_shared<VisualEffect>(program);

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mMesh = mf.CreateSphere(16, 16, 1.0f);
    mMesh->SetEffect(effect);
    mMesh->Update();
    return true;
}

void PlaneMeshIntersectionWindow::InitializeCamera()
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

void PlaneMeshIntersectionWindow::UpdateMatrices()
{
    PMIParameters& p = *mPMIParameters->Get<PMIParameters>();
    p.pvMatrix = mCamera->GetProjectionViewMatrix();

#if defined(GTE_USE_MAT_VEC)
    p.wMatrix = mTrackball.GetOrientation() * mMesh->worldTransform;
#else
    p.wMatrix = mMesh->worldTransform * mTrackball.GetOrientation();
#endif

    mEngine->Update(mPMIParameters);
}

