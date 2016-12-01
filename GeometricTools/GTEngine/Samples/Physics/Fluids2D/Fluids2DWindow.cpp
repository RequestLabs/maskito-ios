// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "Fluids2DWindow.h"


Fluids2DWindow::Fluids2DWindow(Parameters& parameters)
    :
    Window(parameters),
    mFluid(mEngine, mProgramFactory, GRID_SIZE, GRID_SIZE, 0.001f, 0.0001f,
        0.0001f)
{
    if (!SetEnvironment() || !CreateOverlay())
    {
        parameters.created = false;
        return;
    }

    mFluid.Initialize();
}

void Fluids2DWindow::OnIdle()
{
    mTimer.Measure();

    mFluid.DoSimulationStep();
    mEngine->Draw(mOverlay);
    mEngine->Draw(8, mYSize - 8, { 1.0f, 1.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(1);

#if defined(SAVE_RENDERING_TO_DISK)
    mEngine->Enable(mTarget);
    mEngine->ClearBuffers();
    mEngine->Draw(mOverlay);
    mEngine->Disable(mTarget);
    mEngine->CopyGpuToCpu(mTarget->GetRTTexture(0));
    WICFileIO::SaveToPNG(
        "Video/Smoke" + std::to_string(mVideoFrame) + ".png",
        mTarget->GetRTTexture(0).get());
    ++mVideoFrame;
#endif

    mTimer.UpdateFrameCount();
}

bool Fluids2DWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case '0':
        mFluid.Initialize();
        return true;
    }

    return Window::OnCharPress(key, x, y);
}

bool Fluids2DWindow::SetEnvironment()
{
    std::string path = mEnvironment.GetVariable("GTE_PATH");
    if (path == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Physics/Fluids2D/Shaders/");

    if (mEnvironment.GetPath("DrawDensity.hlsl") == "")
    {
        LogError("Cannot find file DrawDensity.hlsl.");
        return false;
    }

    return true;
}

bool Fluids2DWindow::CreateOverlay()
{
    // Create the supporting objects for visualizing the fluid simulation.
    std::string drawDensityPSSource = ProgramFactory::GetStringFromFile(
        mEnvironment.GetPath("DrawDensity.hlsl"));
    mOverlay = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, GRID_SIZE, GRID_SIZE, drawDensityPSSource);
    std::shared_ptr<SamplerState> bilinearClampSampler(new SamplerState());
    bilinearClampSampler->filter = SamplerState::MIN_L_MAG_L_MIP_P;
    bilinearClampSampler->mode[0] = SamplerState::CLAMP;
    bilinearClampSampler->mode[1] = SamplerState::CLAMP;
    std::shared_ptr<PixelShader> pshader =
        mOverlay->GetProgram()->GetPShader();
    pshader->Set("state", mFluid.GetState());
    pshader->Set("bilinearClampSampler", bilinearClampSampler);

    mNoDepthState = std::make_shared<DepthStencilState>();
    mNoDepthState->depthEnable = false;
    mEngine->SetDepthStencilState(mNoDepthState);
    mNoCullingState = std::make_shared<RasterizerState>();
    mNoCullingState->cullMode = RasterizerState::CULL_NONE;
    mEngine->SetRasterizerState(mNoCullingState);

#if defined(SAVE_RENDERING_TO_DISK)
    mTarget = std::make_shared<DrawTarget>(1, DF_R8G8B8A8_UNORM, mXSize,
        mYSize);
    mTarget->GetRTTexture(0)->SetCopyType(Resource::COPY_STAGING_TO_CPU);
    mVideoFrame = 0;
#endif
    return true;
}

