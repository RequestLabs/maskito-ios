// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "ShortestPathWindow.h"


ShortestPathWindow::ShortestPathWindow(Parameters& parameters)
    :
    Window(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    CreateWeightsShader();

#if defined(USE_CPU_SHORTEST_PATH)
    mCpuShortestPath = std::make_shared<CpuShortestPath>(mWeights);
#else
    bool created = false;
    mGpuShortestPath = std::make_shared<GpuShortestPath>(mProgramFactory,
        mWeights, mEnvironment, created);
    if (!created)
    {
        // LogError calls were made in the GpuShortestPath constructor.
        parameters.created = false;
        return;
    }
#endif

    mOverlay = std::make_shared<OverlayEffect>(mProgramFactory, ISIZE,
        ISIZE, ISIZE, ISIZE, SamplerState::MIN_P_MAG_P_MIP_P,
        SamplerState::CLAMP, SamplerState::CLAMP, true);
    mOverlay->SetTexture(mWeights);
}

void ShortestPathWindow::OnIdle()
{
    mTimer.Measure();

    GenerateWeights();

    std::stack<std::pair<int, int>> path;
#if defined(USE_CPU_SHORTEST_PATH)
    mCpuShortestPath->Compute(path);
#else
    mGpuShortestPath->Compute(mEngine, path);
#endif
    DrawPath(path);

    mEngine->Draw(mOverlay);
    mEngine->Draw(8, mYSize - 8, { 1.0f, 1.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool ShortestPathWindow::SetEnvironment()
{
    std::string path = mEnvironment.GetVariable("GTE_PATH");
    if (path == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Geometrics/ShortestPath/Shaders/");

    if (mEnvironment.GetPath("WeightsShader.hlsl") == "")
    {
        LogError("Cannot find file WeightsShader.hlsl.");
        return false;
    }

    return true;
}

void ShortestPathWindow::CreateWeightsShader()
{
    // Perturb the smooth bicubic surface to avoid having a shortest path of
    // a small number of line segments.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.1f, 0.5f);
    mRandom = std::make_shared<Texture2>(DF_R32_FLOAT, ISIZE, ISIZE);
    float* random = mRandom->Get<float>();
    for (int i = 0; i < ISIZE*ISIZE; ++i)
    {
        random[i] += rnd(mte);
    }

    mWeights = std::make_shared<Texture2>(DF_R32G32B32A32_FLOAT, ISIZE,
        ISIZE);
    mWeights->SetUsage(Resource::SHADER_OUTPUT);
    mWeights->SetCopyType(Resource::COPY_BIDIRECTIONAL);

    std::string path = mEnvironment.GetPath("WeightsShader.hlsl");
    int const numThreads = 8;
    mNumGroups = ISIZE / numThreads;
    mProgramFactory.defines.Set("NUM_X_THREADS", numThreads);
    mProgramFactory.defines.Set("NUM_Y_THREADS", numThreads);
    // TODO: Trap whether mWeightsProgram is null and respond accordingly.
    mWeightsProgram = mProgramFactory.CreateFromFile(path);
    std::shared_ptr<ComputeShader> cshader = mWeightsProgram->GetCShader();
    cshader->Set("ControlPoints", CreateBicubicMatrix());
    cshader->Set("random", mRandom);
    cshader->Set("weights", mWeights);
    mProgramFactory.defines.Clear();
}

void ShortestPathWindow::GenerateWeights()
{
    // Generate the height field as gray scale.  The shortest path computed
    // on the GPU will be overlaid in color.
    mEngine->Execute(mWeightsProgram->GetCShader(), mNumGroups,
        mNumGroups, 1);

    // Get a CPU copy of the weights to compute the shortest path using
    // CPU code.
    mEngine->CopyGpuToCpu(mWeights);
}

std::shared_ptr<ConstantBuffer> ShortestPathWindow::CreateBicubicMatrix()
{
    // Generate random samples for the bicubic Bezier surface.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.05f, 1.0f);
    float P[4][4];
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            P[r][c] = rnd(mte);
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
        +25.0f*P[0][0] - 90.0f*P[0][1] + 45.0f*P[0][2] - 10.0f*P[0][3]
        - 90.0f*P[1][0] + 324.0f*P[1][1] - 162.0f*P[1][2] + 36.0f*P[1][3]
        + 45.0f*P[2][0] - 162.0f*P[2][1] + 81.0f*P[2][2] - 18.0f*P[2][3]
        - 10.0f*P[3][0] + 36.0f*P[3][1] - 18.0f*P[3][2] + 4.0f*P[3][3]
        ) / 36.0f;
    control[1][2] = (
        -10.0f*P[0][0] + 45.0f*P[0][1] - 90.0f*P[0][2] + 25.0f*P[0][3]
        + 36.0f*P[1][0] - 162.0f*P[1][1] + 324.0f*P[1][2] - 90.0f*P[1][3]
        - 18.0f*P[2][0] + 81.0f*P[2][1] - 162.0f*P[2][2] + 45.0f*P[2][3]
        + 4.0f*P[3][0] - 18.0f*P[3][1] + 36.0f*P[3][2] - 10.0f*P[3][3]
        ) / 36.0f;
    control[1][3] = (
        -5.0f*P[0][3] + 18.0f*P[1][3] - 9.0f*P[2][3] + 2.0f*P[3][3]
        ) / 6.0f;
    control[2][0] = (
        +2.0f*P[0][0] - 9.0f*P[1][0] + 18.0f*P[2][0] - 5.0f*P[3][0]
        ) / 6.0f;
    control[2][1] = (
        -10.0f*P[0][0] + 36.0f*P[0][1] - 18.0f*P[0][2] + 4.0f*P[0][3]
        + 45.0f*P[1][0] - 162.0f*P[1][1] + 81.0f*P[1][2] - 18.0f*P[1][3]
        - 90.0f*P[2][0] + 324.0f*P[2][1] - 162.0f*P[2][2] + 36.0f*P[2][3]
        + 25.0f*P[3][0] - 90.0f*P[3][1] + 45.0f*P[3][2] - 10.0f*P[3][3]
        ) / 36.0f;
    control[2][2] = (
        +4.0f*P[0][0] - 18.0f*P[0][1] + 36.0f*P[0][2] - 10.0f*P[0][3]
        - 18.0f*P[1][0] + 81.0f*P[1][1] - 162.0f*P[1][2] + 45.0f*P[1][3]
        + 36.0f*P[2][0] - 162.0f*P[2][1] + 324.0f*P[2][2] - 90.0f*P[2][3]
        - 10.0f*P[3][0] + 45.0f*P[3][1] - 90.0f*P[3][2] + 25.0f*P[3][3]
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

    std::shared_ptr<ConstantBuffer> controlBuffer(new ConstantBuffer(
        16*sizeof(Vector4<float>), false));
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

void ShortestPathWindow::DrawPath(std::stack<std::pair<int, int>>& path)
{
    Vector4<float>* texels = mWeights->Get<Vector4<float>>();
    std::pair<int, int> loc0 = path.top();
    path.pop();
    while (path.size() > 0)
    {
        std::pair<int, int> loc1 = path.top();
        path.pop();

        ImageUtility2::DrawLine(
            loc0.first, loc0.second, loc1.first, loc1.second,
            [this, texels](int x, int y)
        {
            texels[x + ISIZE*y] = { 1.0f, 0.0f, 0.0f, 1.0f };
        }
        );

        loc0 = loc1;
    }
    mEngine->CopyCpuToGpu(mWeights);
}

