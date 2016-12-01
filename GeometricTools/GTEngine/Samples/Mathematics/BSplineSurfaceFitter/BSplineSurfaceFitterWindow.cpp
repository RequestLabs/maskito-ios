// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "BSplineSurfaceFitterWindow.h"


BSplineSurfaceFitterWindow::BSplineSurfaceFitterWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cullMode = RasterizerState::CULL_NONE;

    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cullMode = RasterizerState::CULL_NONE;
    mNoCullWireState->fillMode = RasterizerState::FILL_WIREFRAME;

    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::BM_SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::BM_INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::BM_SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::BM_INV_SRC_ALPHA;

    mEngine->SetRasterizerState(mNoCullState);
    mEngine->SetClearColor({ 0.0f, 0.5f, 0.75f, 1.0f });

    CreateScene();
    InitializeCamera();
}

void BSplineSurfaceFitterWindow::OnDisplay()
{
    mCameraRig.Move();
    mEngine->ClearBuffers();
    mEngine->Draw(mHeightField);
    mEngine->SetBlendState(mBlendState);
    mEngine->Draw(mFittedField);
    mEngine->SetDefaultBlendState();
    mEngine->DisplayColorBuffer(0);
}

void BSplineSurfaceFitterWindow::OnIdle()
{
    if (mCameraRig.Move())
    {
        OnDisplay();
    }
}

bool BSplineSurfaceFitterWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mNoCullState == mEngine->GetRasterizerState())
        {
            mEngine->SetRasterizerState(mNoCullWireState);
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullState);
        }
        OnDisplay();
        return true;
    }

    return Window::OnCharPress(key, x, y);
}

bool BSplineSurfaceFitterWindow::OnMouseClick(MouseButton button,
    MouseState state, int x, int y, unsigned int modifiers)
{
    if (Window3::OnMouseClick(button, state, x, y, modifiers))
    {
        OnDisplay();
    }
    return true;
}

bool BSplineSurfaceFitterWindow::OnMouseMotion(MouseButton button, int x,
    int y, unsigned int modifiers)
{
    if (Window3::OnMouseMotion(button, x, y, modifiers))
    {
        OnDisplay();
    }
    return true;
}

bool BSplineSurfaceFitterWindow::SetEnvironment()
{
    std::string path = mEnvironment.GetVariable("GTE_PATH");
    if (path == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return false;
    }
    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("BTHeightField.png") == "")
    {
        LogError("Cannot find file BTHeightField.png.");
        return false;
    }

    return true;
}

void BSplineSurfaceFitterWindow::CreateScene()
{
    // Begin with a flat 64x64 height field.
    int const numSamples = 64;
    float const extent = 8.0f;
    VertexFormat hfformat;
    hfformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    hfformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(hfformat);
    mHeightField = mf.CreateRectangle(numSamples, numSamples, extent, extent);
    int numVertices = numSamples * numSamples;
    VertexPT* hfvertices = mHeightField->GetVertexBuffer()->Get<VertexPT>();

    // Set the heights based on a precomputed height field.  Also create a
    // texture image to go with the height field.
    std::string path = mEnvironment.GetPath("BTHeightField.png");
    std::shared_ptr<Texture2> texture(WICFileIO::Load(path, false));
    std::shared_ptr<Texture2Effect> txeffect =
        std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::CLAMP,
        SamplerState::CLAMP);
    mHeightField->SetEffect(txeffect);

    std::mt19937 mte;
    std::uniform_real_distribution<float> symmr(-0.05f, 0.05f);
    std::uniform_real_distribution<float> intvr(32.0f, 64.0f);
    unsigned char* data = (unsigned char*)texture->Get<unsigned char>();
    std::vector<Vector3<float>> samplePoints(numVertices);
    for (int i = 0; i < numVertices; ++i)
    {
        unsigned char value = *data;
        float height = 3.0f*((float)value) / 255.0f + symmr(mte);
        *data++ = (unsigned char)intvr(mte);
        *data++ = 3 * (128 - value / 2) / 4;
        *data++ = 0;
        data++;

        hfvertices[i].position[2] = height;
        samplePoints[i] = hfvertices[i].position;
    }

    // Compute a B-Spline surface with NxN control points, where N < 64.
    // This surface will be sampled to 64x64 and displayed together with the
    // original height field for comparison.
    int const numControls = 32;
    int const degree = 3;
    BSplineSurfaceFit<float> fitter(degree, numControls, numSamples, degree,
        numControls, numSamples, &samplePoints[0]);

    VertexFormat ffformat;
    ffformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    ffformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);
    mf.SetVertexFormat(ffformat);
    mFittedField = mf.CreateRectangle(numSamples, numSamples, extent, extent);
    VertexPC* ffvertices = mFittedField->GetVertexBuffer()->Get<VertexPC>();

    Vector4<float> translucent{ 1.0f, 1.0f, 1.0f, 0.5f };
    for (int i = 0; i < numVertices; ++i)
    {
        float u = 0.5f*(ffvertices[i].position[0] / extent + 1.0f);
        float v = 0.5f*(ffvertices[i].position[1] / extent + 1.0f);
        ffvertices[i].position = fitter.GetPosition(u, v);
        ffvertices[i].color = translucent;
    }

    std::shared_ptr<VertexColorEffect> vceffect =
        std::make_shared<VertexColorEffect>(mProgramFactory);
    mFittedField->SetEffect(vceffect);

    mCameraRig.Subscribe(mHeightField->worldTransform,
        txeffect->GetPVWMatrixConstant());
    mCameraRig.Subscribe(mFittedField->worldTransform,
        vceffect->GetPVWMatrixConstant());

    mTrackball.Attach(mHeightField);
    mTrackball.Attach(mFittedField);
    mTrackball.Update();
}

void BSplineSurfaceFitterWindow::InitializeCamera()
{
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 0.01f, 100.0f);
    Vector4<float> camPosition{ 0.0f, -9.0f, 1.5f, 1.0f };
    Vector4<float> camDVector{ 0.0f, 1.0f, 0.0f, 0.0f };
    Vector4<float> camUVector{ 0.0f, 0.0f, 1.0f, 0.0f };
    Vector4<float> camRVector = Cross(camDVector, camUVector);
    mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

    mTranslationSpeed = 0.005f;
    mRotationSpeed = 0.002f;
    mTranslationSpeedFactor = 2.0f;
    mRotationSpeedFactor = 2.0f;
    mCameraRig.SetTranslationSpeed(mTranslationSpeed);
    mCameraRig.SetRotationSpeed(mRotationSpeed);
    mCameraRig.ComputeWorldAxes();
    mCameraRig.UpdatePVWMatrices();
}

