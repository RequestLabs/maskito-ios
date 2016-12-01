// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "GeometryShadersWindow.h"


GeometryShadersWindow::GeometryShadersWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment() || !CreateScene())
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({ 1.0f, 1.0f, 1.0f, 1.0f });
    InitializeCamera();

    mCamera->SetFrustum(60.0f, GetAspectRatio(), 0.1f, 100.0f);
    Vector4<float> camPosition{ 2.8f, 0.0f, 0.0f, 1.0f };
    Vector4<float> camDVector{ -1.0f, 0.0f, 0.0f, 0.0f };
    Vector4<float> camUVector{ 0.0f, 0.0f, 1.0f, 0.0f };
    Vector4<float> camRVector = Cross(camDVector, camUVector);
    mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

#if defined(SAVE_RENDERING_TO_DISK)
    mTarget = std::make_shared<DrawTarget>(1, DF_R8G8B8A8_UNORM, mXSize,
        mYSize);
    mTarget->GetRTTexture(0)->SetCopyType(Resource::COPY_STAGING_TO_CPU);
#endif
}

void GeometryShadersWindow::OnIdle()
{
    mTimer.Measure();

    mCameraRig.Move();
    UpdateConstants();

    mEngine->ClearBuffers();
    mEngine->Draw(mMesh);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

#if defined(SAVE_RENDERING_TO_DISK)
    mEngine->Enable(mTarget);
    mEngine->ClearBuffers();
    mEngine->Draw(mMesh);
    mEngine->Disable(mTarget);
    mEngine->CopyGpuToCpu(mTarget->GetRTTexture(0));
    WICFileIO::SaveToPNG("GeometryShaders.png",
        mTarget->GetRTTexture(0).get());
#endif

    mTimer.UpdateFrameCount();
}

bool GeometryShadersWindow::SetEnvironment()
{
    std::string path = mEnvironment.GetVariable("GTE_PATH");
    if (path == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Graphics/GeometryShaders/Shaders/");

    if (mEnvironment.GetPath("RandomSquares.hlsl") == "")
    {
        LogError("Cannot find file RandomSquares.hlsl.");
        return false;
    }

    if (mEnvironment.GetPath("RandomSquaresIndirect.hlsl") == "")
    {
        LogError("Cannot find file RandomSquaresIndirect.hlsl.");
        return false;
    }

    return true;
}

bool GeometryShadersWindow::CreateScene()
{
    std::string filename;
#if defined(USE_DRAW_DIRECT)
    filename = mEnvironment.GetPath("RandomSquares.hlsl");
#else
    filename = mEnvironment.GetPath("RandomSquaresIndirect.hlsl");
#endif
    std::shared_ptr<VisualProgram> program =
        mProgramFactory.CreateFromFiles(filename, filename, filename);
    if (!program)
    {
        return false;
    }

    // Create particles used by direct and indirect drawing.
    struct Vertex
    {
        Vector3<float> position;
        Vector3<float> color;
        float size;
    };

    // Use a Mersenne twister engine for random numbers.
    std::mt19937 mte;
    std::uniform_real_distribution<float> symr(-1.0f, 1.0f);
    std::uniform_real_distribution<float> unir(0.0f, 1.0f);
    std::uniform_real_distribution<float> posr(0.01f, 0.1f);

    int const numParticles = 128;
    std::vector<Vertex> particles(numParticles);
    for (auto& particle : particles)
    {
        particle.position = { symr(mte), symr(mte), symr(mte) };
        particle.color = { unir(mte), unir(mte), unir(mte) };
        particle.size = posr(mte);
    }

    // Create the constant buffer used by direct and indirect drawing.
    mMatrices = std::make_shared<ConstantBuffer>(
        2 * sizeof(Matrix4x4<float>), true);
    program->GetGShader()->Set("Matrices", mMatrices);

#if defined(USE_DRAW_DIRECT)
    // Create a mesh for direct drawing.
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_COLOR, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32_FLOAT, 0);
    std::shared_ptr<VertexBuffer> vbuffer(new VertexBuffer(vformat,
        numParticles));
    Memcpy(vbuffer->GetData(), &particles[0], numParticles*sizeof(Vertex));
#else
    // Create a mesh for indirect drawing.
    std::shared_ptr<VertexBuffer> vbuffer(new VertexBuffer(numParticles));
    mParticles = std::make_shared<StructuredBuffer>(numParticles,
        sizeof(Vertex));
    Memcpy(mParticles->GetData(), &particles[0], numParticles*sizeof(Vertex));
    gshader->Set("particles", mParticles);
#endif

    std::shared_ptr<IndexBuffer> ibuffer(new IndexBuffer(IP_POLYPOINT,
        numParticles));

    std::shared_ptr<VisualEffect> effect =
        std::make_shared<VisualEffect>(program);

    mMesh = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    return true;
}

void GeometryShadersWindow::InitializeCamera()
{
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 0.1f, 100.0f);
    Vector4<float> camPosition{ 2.8f, 0.0f, 0.0f, 1.0f };
    Vector4<float> camDVector{ -1.0f, 0.0f, 0.0f, 0.0f };
    Vector4<float> camUVector{ 0.0f, 0.0f, 1.0f, 0.0f };
    Vector4<float> camRVector = Cross(camDVector, camUVector);
    mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

    mTranslationSpeed = 0.01f;
    mRotationSpeed = 0.001f;
    mTranslationSpeedFactor = 2.0f;
    mRotationSpeedFactor = 2.0f;
    mCameraRig.SetTranslationSpeed(mTranslationSpeed);
    mCameraRig.SetRotationSpeed(mRotationSpeed);
    mCameraRig.ComputeWorldAxes();
}

void GeometryShadersWindow::UpdateConstants()
{
    Matrix4x4<float> vwMatrix, pMatrix;
#if defined(GTE_USE_MAT_VEC)
    vwMatrix = mCamera->GetViewMatrix() * mTrackball.GetOrientation();
#else
    vwMatrix = mTrackball.GetOrientation() * mCamera->GetViewMatrix();
#endif

    mMatrices->SetMember("vwMatrix", vwMatrix);
    mMatrices->SetMember("pMatrix", mCamera->GetProjectionMatrix());
    mEngine->Update(mMatrices);
}

