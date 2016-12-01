// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "Fluids3DWindow.h"


Fluids3DWindow::Fluids3DWindow(Parameters& parameters)
    :
    Window3(parameters),
    mFluid(mEngine, mProgramFactory, GRID_SIZE, GRID_SIZE, GRID_SIZE, 0.002f)
{
    if (!SetEnvironment() || !CreateNestedBoxes())
    {
        parameters.created = false;
        return;
    }

    // Use blending for the visualization.
    mAlphaState = std::make_shared<BlendState>();
    mAlphaState->target[0].enable = true;
    mAlphaState->target[0].srcColor = BlendState::BM_SRC_ALPHA;
    mAlphaState->target[0].dstColor = BlendState::BM_INV_SRC_ALPHA;
    mAlphaState->target[0].srcAlpha = BlendState::BM_SRC_ALPHA;
    mAlphaState->target[0].dstAlpha = BlendState::BM_INV_SRC_ALPHA;
    mEngine->SetBlendState(mAlphaState);

    // The alpha channel must be zero for the blending of density to work
    // correctly through the fluid region.
    mEngine->SetClearColor({ 1.0f, 1.0f, 1.0f, 0.0f });

    // The geometric proxies for volume rendering are concentric boxes.  They
    // are drawn from inside to outside for correctly sorted drawing, so depth
    // buffering is not needed.
    mNoDepthState = std::make_shared<DepthStencilState>();
    mNoDepthState->depthEnable = false;
    mEngine->SetDepthStencilState(mNoDepthState);

    mFluid.Initialize();
    InitializeCamera();
    UpdateConstants();
}

void Fluids3DWindow::OnIdle()
{
    mTimer.Measure();

    mCameraRig.Move();
    UpdateConstants();

    mFluid.DoSimulationStep();

    mEngine->ClearBuffers();
    for (auto visual : mVisible)
    {
        mEngine->Draw(visual);
    }
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(1);

    mTimer.UpdateFrameCount();
}

bool Fluids3DWindow::OnCharPress(unsigned char key, int x, int y)
{
    if (key == '0')
    {
        mFluid.Initialize();
        return true;
    }

    return Window::OnCharPress(key, x, y);
}

bool Fluids3DWindow::SetEnvironment()
{
    std::string path = mEnvironment.GetVariable("GTE_PATH");
    if (path == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Physics/Fluids3D/Shaders/");

    if (mEnvironment.GetPath("VolumeRender.hlsl") == "")
    {
        LogError("Cannot find file VolumeRender.hlsl.");
        return false;
    }

    return true;
}

bool Fluids3DWindow::CreateNestedBoxes()
{
    std::string path = mEnvironment.GetPath("VolumeRender.hlsl");
    std::shared_ptr<VisualProgram> program =
        mProgramFactory.CreateFromFiles(path, path, "");
    if (!program)
    {
        return false;
    }

    mPVWMatrixBuffer = std::make_shared<ConstantBuffer>(
        sizeof(Matrix4x4<float>), true);
    *mPVWMatrixBuffer->Get<Matrix4x4<float>>() = Matrix4x4<float>::Identity();

    mTrilinearClampSampler = std::make_shared<SamplerState>();
    mTrilinearClampSampler->filter = SamplerState::MIN_L_MAG_L_MIP_P;
    mTrilinearClampSampler->mode[0] = SamplerState::CLAMP;
    mTrilinearClampSampler->mode[1] = SamplerState::CLAMP;
    mTrilinearClampSampler->mode[2] = SamplerState::CLAMP;

    program->GetVShader()->Set("PVWMatrix", mPVWMatrixBuffer);
    program->GetPShader()->Set("volumeTexture", mFluid.GetState());
    program->GetPShader()->Set("trilinearClampSampler",
        mTrilinearClampSampler);

    std::shared_ptr<VisualEffect> effect =
        std::make_shared<VisualEffect>(program);

    struct Vertex { Vector3<float> position, tcoord; };
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32B32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    int const numBoxes = 128;
    for (int i = 1; i <= numBoxes; ++i)
    {
        float extent = 0.5f*i/(numBoxes - 1.0f);
        std::shared_ptr<Visual> visual(mf.CreateBox(extent, extent, extent));
        VertexBuffer* vbuffer = visual->GetVertexBuffer().get();
        Vertex* vertex = vbuffer->Get<Vertex>();
        for (unsigned int j = 0; j < vbuffer->GetNumElements(); ++j, ++vertex)
        {
            Vector3<float>& tcd = vertex->tcoord;
            Vector3<float> pos = vertex->position;
            Vector4<float> tmp{ pos[0] + 0.5f, pos[1] + 0.5f, pos[2] + 0.5f,
                0.0f };
            for (int k = 0; k < 3; ++k)
            {
                tcd[k] = 0.5f*(tmp[k] + 1.0f);
            }
        }

        visual->SetEffect(effect);
        mVisible.push_back(visual);
    }

    return true;
}

void Fluids3DWindow::InitializeCamera()
{
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 0.01f, 100.0f);
    Vector4<float> camPosition{ 0.0f, 0.0f, -2.25f, 1.0f };
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

void Fluids3DWindow::UpdateConstants()
{
    Matrix4x4<float> pvMatrix = mCamera->GetProjectionViewMatrix();
    Matrix4x4<float>& pvwMatrix = *mPVWMatrixBuffer->Get<Matrix4x4<float>>();

#if defined(GTE_USE_MAT_VEC)
    pvwMatrix = pvMatrix * mTrackball.GetOrientation();
#else
    pvwMatrix = mTrackball.GetOrientation() * pvMatrix;
#endif

    mEngine->Update(mPVWMatrixBuffer);
}

