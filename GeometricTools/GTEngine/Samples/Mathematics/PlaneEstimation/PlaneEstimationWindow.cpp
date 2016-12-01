// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "PlaneEstimationWindow.h"


PlaneEstimationWindow::PlaneEstimationWindow (Parameters& parameters)
    :
    Window(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    // Create the shaders.
    int const numThreads = 8;
    mProgramFactory.defines.Set("NUM_X_THREADS", numThreads);
    mProgramFactory.defines.Set("NUM_Y_THREADS", numThreads);
    mProgramFactory.defines.Set("RADIUS", 3);

    mPositionProgram = mProgramFactory.CreateFromFile(
        mEnvironment.GetPath("EvaluateBezier.hlsl"));
    if (!mPositionProgram)
    {
        parameters.created = false;
        return;
    }

    mPlaneProgram = mProgramFactory.CreateFromFile(
        mEnvironment.GetPath("PlaneEstimation.hlsl"));
    if (!mPlaneProgram)
    {
        parameters.created = false;
        return;
    }

    // Create and attach resources to the shaders.
    std::shared_ptr<ComputeShader> cshader = mPositionProgram->GetCShader();
    cshader->Set("ControlPoints", CreateBezierControls());

    int const txWidth = 1024, txHeight = 1024;
    mPositions = std::make_shared<Texture2>(DF_R32G32B32A32_FLOAT, txWidth,
        txHeight);
    mPositions->SetUsage(Resource::SHADER_OUTPUT);
    cshader->Set("positions", mPositions);

    mNumXGroups = txWidth / numThreads;
    mNumYGroups = txHeight / numThreads;

    cshader = mPlaneProgram->GetCShader();
    cshader->Set("positions", mPositions);

    mPlanes = std::make_shared<Texture2>(DF_R32G32B32A32_FLOAT, txWidth,
        txHeight);
    mPlanes->SetUsage(Resource::SHADER_OUTPUT);
    cshader->Set("planes", mPlanes);

    std::shared_ptr<SamplerState> sstate(new SamplerState());
    sstate->filter = SamplerState::MIN_L_MAG_L_MIP_P;
    sstate->mode[0] = SamplerState::CLAMP;
    sstate->mode[1] = SamplerState::CLAMP;

    std::string posPSSource = ProgramFactory::GetStringFromFile(
        mEnvironment.GetPath("PositionVisualize.hlsl"));
    mOverlay[0] = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, txWidth, txHeight, posPSSource);
    std::array<int, 4> rect0 = { 0, 0, mXSize / 2, mYSize };
    mOverlay[0]->SetOverlayRectangle(rect0);
    std::shared_ptr<PixelShader> pshader =
        mOverlay[0]->GetProgram()->GetPShader();
    pshader->Set("myTexture", mPositions);
    pshader->Set("mySampler", sstate);

    std::string plnPSSource = ProgramFactory::GetStringFromFile(
        mEnvironment.GetPath("PlaneVisualize.hlsl"));
    mOverlay[1] = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, txWidth, txHeight, plnPSSource);
    pshader = mOverlay[1]->GetProgram()->GetPShader();
    std::array<int, 4> rect1 = { mXSize / 2, 0, mXSize / 2, mYSize };
    mOverlay[1]->SetOverlayRectangle(rect1);
    pshader->Set("myTexture", mPlanes);
    pshader->Set("mySampler", sstate);
}

void PlaneEstimationWindow::OnIdle ()
{
    mTimer.Measure();

    mEngine->Execute(mPositionProgram->GetCShader(),
        mNumXGroups, mNumYGroups, 1);
    mEngine->Execute(mPlaneProgram->GetCShader(),
        mNumXGroups, mNumYGroups, 1);
    mEngine->Draw(mOverlay[0]);
    mEngine->Draw(mOverlay[1]);
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool PlaneEstimationWindow::SetEnvironment()
{
    std::string path = mEnvironment.GetVariable("GTE_PATH");
    if (path == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return false;
    }
    mEnvironment.Insert(
        path + "/Samples/Mathematics/PlaneEstimation/Shaders/");

    if (mEnvironment.GetPath("EvaluateBezier.hlsl") == "")
    {
        LogError("Cannot find EvaluateBezier.hlsl.");
        return false;
    }

    if (mEnvironment.GetPath("PlaneEstimation.hlsl") == "")
    {
        LogError("Cannot find PlaneEstimation.hlsl.");
        return false;
    }

    if (mEnvironment.GetPath("PlaneVisualize.hlsl") == "")
    {
        LogError("Cannot find PlaneVisualize.hlsl.");
        return false;
    }

    if (mEnvironment.GetPath("PositionVisualize.hlsl") == "")
    {
        LogError("Cannot find PositionVisualize.hlsl.");
        return false;
    }

    return true;
}

std::shared_ptr<ConstantBuffer> PlaneEstimationWindow::CreateBezierControls()
{
    // Generate random samples for the bicubic Bezier surface.  The w-channel
    // is irrelevant, so is set to zero.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(-0.25f, 1.0f);
    float P[4][4];
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            P[r][c]= rnd(mte);
        }
    }

    // Construct the control points from the samples.
    float control[4][4];
    control[0][0] = P[0][0];
    control[0][1] = (
        -5.0f*P[0][0] + 18.0f*P[0][1] - 9.0f*P[0][2] + 2.0f*P[0][3]
        ) / 6.0f;
    control[0][2] = (
        +2.0f*P[0][0] - 9.0f*P[0][1] + 18.0f*P[0][2] - 5.0f*P[0][3]
        ) / 6.0f;
    control[0][3] = P[0][3];
    control[1][0] = (
        -5.0f*P[0][0] + 18.0f*P[1][0] - 9.0f*P[2][0] - 5.0f*P[3][0]
        ) / 6.0f;
    control[1][1] = (
        + 25.0f*P[0][0] -  90.0f*P[0][1] +  45.0f*P[0][2] - 10.0f*P[0][3]
        - 90.0f*P[1][0] + 324.0f*P[1][1] - 162.0f*P[1][2] + 36.0f*P[1][3]
        + 45.0f*P[2][0] - 162.0f*P[2][1] +  81.0f*P[2][2] - 18.0f*P[2][3]
        - 10.0f*P[3][0] +  36.0f*P[3][1] -  18.0f*P[3][2] +  4.0f*P[3][3]
        ) / 36.0f;
    control[1][2] = (
        - 10.0f*P[0][0] +  45.0f*P[0][1] -  90.0f*P[0][2] + 25.0f*P[0][3]
        + 36.0f*P[1][0] - 162.0f*P[1][1] + 324.0f*P[1][2] - 90.0f*P[1][3]
        - 18.0f*P[2][0] +  81.0f*P[2][1] - 162.0f*P[2][2] + 45.0f*P[2][3]
        +  4.0f*P[3][0] -  18.0f*P[3][1] +  36.0f*P[3][2] - 10.0f*P[3][3]
        ) / 36.0f;
    control[1][3] = (
        -5.0f*P[0][3] + 18.0f*P[1][3] - 9.0f*P[2][3] + 2.0f*P[3][3]
        ) / 6.0f;
    control[2][0] = (
        +2.0f*P[0][0] - 9.0f*P[1][0] + 18.0f*P[2][0] - 5.0f*P[3][0]
        ) / 6.0f;
    control[2][1] = (
        - 10.0f*P[0][0] +  36.0f*P[0][1] -  18.0f*P[0][2] +  4.0f*P[0][3]
        + 45.0f*P[1][0] - 162.0f*P[1][1] +  81.0f*P[1][2] - 18.0f*P[1][3]
        - 90.0f*P[2][0] + 324.0f*P[2][1] - 162.0f*P[2][2] + 36.0f*P[2][3]
        + 25.0f*P[3][0] -  90.0f*P[3][1] +  45.0f*P[3][2] - 10.0f*P[3][3]
        ) / 36.0f;
    control[2][2] = (
        +  4.0f*P[0][0] -  18.0f*P[0][1] +  36.0f*P[0][2] - 10.0f*P[0][3]
        - 18.0f*P[1][0] +  81.0f*P[1][1] - 162.0f*P[1][2] + 45.0f*P[1][3]
        + 36.0f*P[2][0] - 162.0f*P[2][1] + 324.0f*P[2][2] - 90.0f*P[2][3]
        - 10.0f*P[3][0] +  45.0f*P[3][1] -  90.0f*P[3][2] + 25.0f*P[3][3]
        ) / 36.0f;
    control[2][3] = (
        +2.0f*P[0][3] - 9.0f*P[1][3] + 18.0f*P[2][3] - 5.0f*P[3][3]
        ) / 6.0f;
    control[3][0] = P[3][0];
    control[3][1] = (
        -5.0f*P[3][0] + 18.0f*P[3][1] - 9.0f*P[3][2] + 2.0f*P[3][3]
        ) / 6.0f;
    control[3][2] = (
        +2.0f*P[3][0] - 9.0f*P[3][1] + 18.0f*P[3][2] - 5.0f*P[3][3]
        ) / 6.0f;
    control[3][3] = P[3][3];

    size_t const numBytes = 16 * sizeof(Vector4<float>);
    std::shared_ptr<ConstantBuffer> controlBuffer(new ConstantBuffer(numBytes,
        false));
    Vector4<float>* data = controlBuffer->Get<Vector4<float>>();
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            // The HLSL constant buffer stores one float per 4-tuple register.
            Vector4<float>& trg = data[c + 4 * r];
            trg[0] = control[r][c];
            trg[1] = 0.0f;
            trg[2] = 0.0f;
            trg[3] = 0.0f;
        }
    }

    return controlBuffer;
}

