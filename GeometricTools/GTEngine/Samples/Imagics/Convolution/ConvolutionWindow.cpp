// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "ConvolutionWindow.h"


ConvolutionWindow::ConvolutionWindow(Parameters& parameters)
    :
    Window(parameters),
    mNumXGroups(0),
    mNumYGroups(0),
    mRadius(1),
    mSelection(0)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    // Load the color image to be convolved.
    std::string path = mEnvironment.GetPath("MedicineBag.jpg");
    Texture2* original = WICFileIO::Load(path, false);
    unsigned int const txWidth = original->GetWidth();
    unsigned int const txHeight = original->GetHeight();

    // Create images for shader inputs and outputs.
    for (int i = 0; i < 3; ++i)
    {
        mImage[i] = std::make_shared<Texture2>(DF_R32G32B32A32_FLOAT, txWidth,
            txHeight);
        mImage[i]->SetUsage(Resource::SHADER_OUTPUT);
    }

    // Map the 8-bit RGBA image to 32-bit RGBA for the numerical convolution.
    unsigned int const* src = original->Get<unsigned int>();
    float* trg = mImage[0]->Get<float>();
    for (unsigned int j = 0; j < txWidth*txHeight; ++j)
    {
        unsigned int rgba = *src++;
        *trg++ = (rgba & 0x000000FF) / 255.0f;
        *trg++ = ((rgba & 0x0000FF00) >> 8) / 255.0f;
        *trg++ = ((rgba & 0x00FF0000) >> 16) / 255.0f;
        *trg++ = 1.0f;
    }
    delete original;

    // Create two overlays, one for the original image and one for the
    // convolved image.
    std::array<int, 4> rect[2] =
    {
        { 0, 0, mXSize / 2, mYSize },
        { mXSize / 2, 0, mXSize / 2, mYSize }
    };
    for (int i = 0; i < 2; ++i)
    {
        mOverlay[i] = std::make_shared<OverlayEffect>(mProgramFactory,
            mXSize, mYSize, txWidth, txHeight,
            SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::CLAMP,
            SamplerState::CLAMP, true);
        mOverlay[i]->SetOverlayRectangle(rect[i]);
        mOverlay[i]->SetTexture(mImage[i]);
    }

    CreateShaders();
}

void ConvolutionWindow::OnIdle()
{
    mTimer.Measure();

    ExecuteShaders();
    mEngine->Draw(mOverlay[0]);
    mEngine->Draw(mOverlay[1]);
    std::string message = "radius = " + std::to_string(mRadius);
    Vector4<float> textColor{ 1.0f, 1.0f, 0.0f, 1.0f };
    mEngine->Draw(8, mYSize - 40, textColor, msName[mSelection]);
    mEngine->Draw(8, mYSize - 24, textColor, message);
    mEngine->Draw(8, mYSize - 8, textColor, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool ConvolutionWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case '0':
        mSelection = 0;
        CreateShaders();
        return true;

    case '1':
        mSelection = 1;
        CreateShaders();
        return true;

    case '2':
        mSelection = 2;
        CreateShaders();
        return true;

    case '3':
        mSelection = 3;
        CreateShaders();
        return true;

    case '4':
        mSelection = 4;
        CreateShaders();
        return true;

    case '+':
    case '=':
        if (mRadius < MAX_RADIUS)
        {
            ++mRadius;
            CreateShaders();
        }
        return true;

    case '-':
    case '_':
        if (mRadius > 1)
        {
            --mRadius;
            CreateShaders();
        }
        return true;
    }

    return Window::OnCharPress(key, x, y);
}

bool ConvolutionWindow::SetEnvironment()
{
    std::string gtpath = mEnvironment.GetVariable("GTE_PATH");
    if (gtpath == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return false;
    }
    mEnvironment.Insert(gtpath + "/Samples/Imagics/Convolution/Shaders/");
    mEnvironment.Insert(gtpath + "/Samples/Data/");

    if (mEnvironment.GetPath("MedicineBag.jpg") == "")
    {
        LogError("Cannot find file MedicineBag.jpg.");
        return false;
    }

    if (mEnvironment.GetPath("Convolve.hlsl") == "")
    {
        LogError("Cannot find file Convolve.hlsl.");
        return false;
    }

    if (mEnvironment.GetPath("ConvolveGS.hlsl") == "")
    {
        LogError("Cannot find file ConvolveGS.hlsl.");
        return false;
    }

    if (mEnvironment.GetPath("ConvolveSeparableH.hlsl") == "")
    {
        LogError("Cannot find file ConvolveSeparableH.hlsl.");
        return false;
    }

    if (mEnvironment.GetPath("ConvolveSeparableHGS.hlsl") == "")
    {
        LogError("Cannot find file ConvolveSeparableHGS.hlsl.");
        return false;
    }

    if (mEnvironment.GetPath("ConvolveSeparableHGS2.hlsl") == "")
    {
        LogError("Cannot find file ConvolveSeparableHGS2.hlsl.");
        return false;
    }

    if (mEnvironment.GetPath("ConvolveSeparableV.hlsl") == "")
    {
        LogError("Cannot find file ConvolveSeparableV.hlsl.");
        return false;
    }

    if (mEnvironment.GetPath("ConvolveSeparableVGS.hlsl") == "")
    {
        LogError("Cannot find file ConvolveSeparableVGS.hlsl.");
        return false;
    }

    if (mEnvironment.GetPath("ConvolveSeparableVGS2.hlsl") == "")
    {
        LogError("Cannot find file ConvolveSeparableVGS2.hlsl.");
        return false;
    }

    return true;
}

void ConvolutionWindow::CreateShaders()
{
    std::string path;
    std::shared_ptr<ComputeShader> cshader;

    switch (mSelection)
    {
    case 0:
        mNumXGroups = mImage[0]->GetWidth() / 16;
        mNumYGroups = mImage[0]->GetHeight() / 16;
        mProgramFactory.defines.Set("NUM_X_THREADS", 16);
        mProgramFactory.defines.Set("NUM_Y_THREADS", 16);
        mProgramFactory.defines.Set("RADIUS", mRadius);
        path = mEnvironment.GetPath("Convolve.hlsl");
        mConvolve = mProgramFactory.CreateFromFile(path);
        if (mConvolve)
        {
            cshader = mConvolve->GetCShader();
            cshader->Set("input", mImage[0]);
            cshader->Set("output", mImage[1]);
            cshader->Set("Weights", GetKernel2(mRadius));
        }
        else
        {
            LogError("Failed to compile Convolve.hlsl.");
        }
        mProgramFactory.defines.Clear();
        break;

    case 1:
        mNumXGroups = mImage[0]->GetWidth() / 16;
        mNumYGroups = mImage[0]->GetHeight() / 16;
        mProgramFactory.defines.Set("RADIUS", mRadius);
        mProgramFactory.defines.Set("NUM_X_THREADS", 16);
        mProgramFactory.defines.Set("NUM_Y_THREADS", 16);
        path = mEnvironment.GetPath("ConvolveGS.hlsl");
        mConvolveGS = mProgramFactory.CreateFromFile(path);
        if (mConvolveGS)
        {
            cshader = mConvolveGS->GetCShader();
            cshader->Set("input", mImage[0]);
            cshader->Set("output", mImage[1]);
            cshader->Set("Weights", GetKernel2(mRadius));
        }
        else
        {
            LogError("Failed to compile ConvolveGS.hlsl.");
        }
        mProgramFactory.defines.Clear();
        break;

    case 2:
        mNumXGroups = mImage[0]->GetWidth() / 16;
        mNumYGroups = mImage[0]->GetHeight() / 16;
        mProgramFactory.defines.Set("NUM_X_THREADS", 16);
        mProgramFactory.defines.Set("NUM_Y_THREADS", 16);
        mProgramFactory.defines.Set("RADIUS", mRadius);
        path = mEnvironment.GetPath("ConvolveSeparableH.hlsl");
        mConvolveSeparableH = mProgramFactory.CreateFromFile(path);
        path = mEnvironment.GetPath("ConvolveSeparableV.hlsl");
        mConvolveSeparableV = mProgramFactory.CreateFromFile(path);
        if (mConvolveSeparableH && mConvolveSeparableV)
        {
            cshader = mConvolveSeparableH->GetCShader();
            cshader->Set("input", mImage[0]);
            cshader->Set("output", mImage[2]);
            cshader->Set("Weights", GetKernel1(mRadius));
            cshader = mConvolveSeparableV->GetCShader();
            cshader->Set("input", mImage[2]);
            cshader->Set("output", mImage[1]);
            cshader->Set("Weights", GetKernel1(mRadius));
        }
        else
        {
            LogError("Failed to compile ConvolveSeparable{H,V}.hlsl.");
        }
        mProgramFactory.defines.Clear();
        break;

    case 3:
        mProgramFactory.defines.Set("RADIUS", mRadius);
        path = mEnvironment.GetPath("ConvolveSeparableHGS.hlsl");
        mConvolveSeparableHGS = mProgramFactory.CreateFromFile(path);
        path = mEnvironment.GetPath("ConvolveSeparableVGS.hlsl");
        mConvolveSeparableVGS = mProgramFactory.CreateFromFile(path);
        if (mConvolveSeparableHGS && mConvolveSeparableVGS)
        {
            cshader = mConvolveSeparableHGS->GetCShader();
            cshader->Set("input", mImage[0]);
            cshader->Set("output", mImage[2]);
            cshader->Set("Weights", GetKernel1(mRadius));
            cshader = mConvolveSeparableVGS->GetCShader();
            cshader->Set("input", mImage[2]);
            cshader->Set("output", mImage[1]);
            cshader->Set("Weights", GetKernel1(mRadius));
        }
        else
        {
            LogError("Failed to compile ConvolveSeparable{H,V}GS.hlsl.");
        }
        mProgramFactory.defines.Clear();
        break;

    case 4:
        mProgramFactory.defines.Set("RADIUS", mRadius);
        path = mEnvironment.GetPath("ConvolveSeparableHGS2.hlsl");
        mConvolveSeparableHGS2 = mProgramFactory.CreateFromFile(path);
        path = mEnvironment.GetPath("ConvolveSeparableVGS2.hlsl");
        mConvolveSeparableVGS2 = mProgramFactory.CreateFromFile(path);
        if (mConvolveSeparableHGS2 && mConvolveSeparableVGS2)
        {
            cshader = mConvolveSeparableHGS2->GetCShader();
            cshader->Set("input", mImage[0]);
            cshader->Set("output", mImage[2]);
            cshader->Set("Weights", GetKernel1(mRadius));
            cshader = mConvolveSeparableVGS2->GetCShader();
            cshader->Set("input", mImage[2]);
            cshader->Set("output", mImage[1]);
            cshader->Set("Weights", GetKernel1(mRadius));
        }
        else
        {
            LogError("Failed to compile ConvolveSeparable{H,V}GS2.hlsl.");
        }
        mProgramFactory.defines.Clear();
        break;
    }
}

void ConvolutionWindow::ExecuteShaders()
{
    switch (mSelection)
    {
    case 0:
        if (mConvolve)
        {
            mEngine->Execute(mConvolve->GetCShader(),
                mNumXGroups, mNumYGroups, 1);
        }
        break;

    case 1:
        if (mConvolveGS)
        {
            mEngine->Execute(mConvolveGS->GetCShader(),
                mNumXGroups, mNumYGroups, 1);
        }
        break;

    case 2:
        if (mConvolveSeparableH && mConvolveSeparableV)
        {
            mEngine->Execute(mConvolveSeparableH->GetCShader(),
                mNumXGroups, mNumYGroups, 1);
            mEngine->Execute(mConvolveSeparableV->GetCShader(),
                mNumXGroups, mNumYGroups, 1);
        }
        break;

    case 3:
        if (mConvolveSeparableHGS && mConvolveSeparableVGS)
        {
            mEngine->Execute(mConvolveSeparableHGS->GetCShader(),
                1, mImage[0]->GetHeight(), 1);
            mEngine->Execute(mConvolveSeparableVGS->GetCShader(),
                mImage[0]->GetWidth(), 1, 1);
        }
        break;

    case 4:
        if (mConvolveSeparableHGS2 && mConvolveSeparableVGS2)
        {
            mEngine->Execute(mConvolveSeparableHGS2->GetCShader(),
                4, mImage[0]->GetHeight(), 1);
            mEngine->Execute(mConvolveSeparableVGS2->GetCShader(),
                mImage[0]->GetWidth(), 4, 1);
        }
        break;
    }
}

std::shared_ptr<ConstantBuffer> ConvolutionWindow::GetKernel1(int radius)
{
    // If radius/sigma = ratio, then exp(-ratio^2/2) = 0.001.
    float const ratio = 3.7169221888498384469524067613045f;
    float sigma = radius / ratio;

    int const numWeights = 2 * radius + 1;
    std::vector<float> weight(numWeights);
    float totalWeight = 0.0f;
    for (int x = -radius, i = 0; x <= radius; ++x, ++i)
    {
        float fx = x / sigma;
        float value = exp(-0.5f*fx*fx);
        weight[i] = value;
        totalWeight += value;
    }

    for (auto& w : weight)
    {
        w /= totalWeight;
    }

    std::shared_ptr<ConstantBuffer> cbuffer(new ConstantBuffer(
        numWeights*sizeof(Vector4<float>), false));
    Vector4<float>* data = cbuffer->Get<Vector4<float>>();
    for (int i = 0; i < numWeights; ++i)
    {
        // The HLSL constant buffer stores one float per 4-tuple register.
        Vector4<float>& entry = data[i];
        entry[0] = weight[i];
        entry[1] = 0.0f;
        entry[2] = 0.0f;
        entry[3] = 0.0f;
    }

    return cbuffer;
}

std::shared_ptr<ConstantBuffer> ConvolutionWindow::GetKernel2(int radius)
{
    // If radius/sigma = ratio, then exp(-ratio^2/2) = 0.001.
    float const ratio = 3.7169221888498384469524067613045f;
    float sigma = radius / ratio;

    int const length = 2 * radius + 1;
    int const numWeights = length * length;
    std::vector<float> weight(numWeights);
    float totalWeight = 0.0f;
    for (int y = -radius, i = 0; y <= radius; ++y)
    {
        float fy = y / sigma;
        for (int x = -radius; x <= radius; ++x, ++i)
        {
            float fx = x / sigma;
            float value = exp(-0.5f*(fx*fx + fy*fy));
            weight[i] = value;
            totalWeight += value;
        }
    }

    for (auto& w : weight)
    {
        w /= totalWeight;
    }

    std::shared_ptr<ConstantBuffer> cbuffer(new ConstantBuffer(
        numWeights*sizeof(Vector4<float>), false));
    Vector4<float>* data = cbuffer->Get<Vector4<float>>();
    for (int i = 0; i < numWeights; ++i)
    {
        // The HLSL constant buffer stores one float per 4-tuple register.
        Vector4<float>& entry = data[i];
        entry[0] = weight[i];
        entry[1] = 0.0f;
        entry[2] = 0.0f;
        entry[3] = 0.0f;
    }

    return cbuffer;
}


std::string ConvolutionWindow::msName[5] =
{
    "convolve",
    "convolve groupshared",
    "convolve separable",
    "convolve separable groupshared (one slice at a time)",
    "convolve separable groupshared (slice processed as subslices)"
};
