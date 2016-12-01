// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "GaussianBlurringWindow.h"


GaussianBlurringWindow::GaussianBlurringWindow(Parameters& parameters)
    :
    Window(parameters),
    mNumXThreads(8),
    mNumYThreads(8),
    mNumXGroups(mXSize / mNumXThreads),  // 1024/8 = 128
    mNumYGroups(mYSize / mNumYThreads),  // 768/8 = 96
    mPass(0)
{
    if (!SetEnvironment() || !CreateImages() || !CreateShader())
    {
        parameters.created = false;
        return;
    }

    // Create an overlay that covers the entire window.  The blurred image
    // is drawn by the overlay effect.
    mOverlay = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, mXSize, mYSize, SamplerState::MIN_P_MAG_P_MIP_P,
        SamplerState::CLAMP, SamplerState::CLAMP, true);
    mOverlay->SetTexture(mImage[1]);

#if defined(SAVE_RENDERING_TO_DISK)
    mTarget = std::make_shared<DrawTarget>(1, DF_R8G8B8A8_UNORM, mXSize,
        mYSize);
    mTarget->GetRTTexture(0)->SetCopyType(Resource::COPY_STAGING_TO_CPU);
#endif
}

void GaussianBlurringWindow::OnIdle()
{
    mTimer.Measure();

#if defined(SAVE_RENDERING_TO_DISK)
    if (mPass == 0 || mPass == 100 || mPass == 1000 || mPass == 10000)
    {
        mOverlay->SetTexture(mImage[0]);
        mEngine->SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
        mEngine->ClearBuffers();
        mEngine->Enable(mTarget);
        mEngine->Draw(mOverlay);
        mEngine->Disable(mTarget);
        mEngine->CopyGpuToCpu(mTarget->GetRTTexture(0));
        WICFileIO::SaveToPNG("Gauss" + std::to_string(mPass) + ".png",
            mTarget->GetRTTexture(0).get());
        mOverlay->SetTexture(mImage[1]);
    }
#endif

    std::shared_ptr<ComputeShader> cshader =
        mGaussianBlurProgram->GetCShader();
    mEngine->Execute(cshader, mNumXGroups, mNumYGroups, 1);
    mEngine->Draw(mOverlay);
    std::swap(mImage[0], mImage[1]);
    cshader->Set("input", mImage[0]);
    cshader->Set("output", mImage[1]);
    mOverlay->SetTexture(mImage[1]);
    ++mPass;

    mEngine->Draw(8, mYSize - 8, { 1.0f, 1.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool GaussianBlurringWindow::SetEnvironment()
{
    std::string path = mEnvironment.GetVariable("GTE_PATH");
    if (path == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Imagics/GaussianBlurring/Shaders/");
    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("MedicineBag.jpg") == "")
    {
        LogError("Cannot find file MedicineBag.jpg.");
        return false;
    }

    if (mEnvironment.GetPath("GaussianBlur3x3.hlsl") == "")
    {
        LogError("Cannot find file GaussianBlur3x3.hlsl.");
        return false;
    }

    return true;
}

bool GaussianBlurringWindow::CreateImages()
{
    Texture2* original = WICFileIO::Load(
        mEnvironment.GetPath("MedicineBag.jpg"), false);
    if (!original)
    {
        return false;
    }

    for (int i = 0; i < 2; ++i)
    {
        mImage[i] = std::make_shared<Texture2>(DF_R32G32B32A32_FLOAT, mXSize,
            mYSize);
        mImage[i]->SetUsage(Resource::SHADER_OUTPUT);
    }

    unsigned int const* src = original->Get<unsigned int>();
    float* trg = mImage[0]->Get<float>();
    for (int j = 0; j < mXSize*mYSize; ++j)
    {
        unsigned int rgba = *src++;
        *trg++ = (rgba & 0x000000FF) / 255.0f;
        *trg++ = ((rgba & 0x0000FF00) >> 8) / 255.0f;
        *trg++ = ((rgba & 0x00FF0000) >> 16) / 255.0f;
        *trg++ = 1.0f;
    }
    delete original;

    return true;
}

bool GaussianBlurringWindow::CreateShader()
{
#if !defined(GTE_DEV_OPENGL)
    // The flags are chosen to allow you to debug the shaders through MSVS.
    // The menu path is "Debug | Graphics | Start Diagnostics" (ALT+F5).
    mProgramFactory.PushFlags();
    mProgramFactory.flags =
        D3DCOMPILE_ENABLE_STRICTNESS |
        D3DCOMPILE_IEEE_STRICTNESS |
        D3DCOMPILE_DEBUG |
        D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    mProgramFactory.defines.Set("NUM_X_THREADS", mNumXThreads);
    mProgramFactory.defines.Set("NUM_Y_THREADS", mNumYThreads);
    mGaussianBlurProgram = mProgramFactory.CreateFromFile(
        mEnvironment.GetPath("GaussianBlur3x3.hlsl"));
    mProgramFactory.defines.Clear();

#if !defined(GTE_DEV_OPENGL)
    mProgramFactory.PopFlags();
#endif

    if (mGaussianBlurProgram)
    {
        mGaussianBlurProgram->GetCShader()->Set("input", mImage[0]);
        mGaussianBlurProgram->GetCShader()->Set("output", mImage[1]);
        return true;
    }
    return false;
}

