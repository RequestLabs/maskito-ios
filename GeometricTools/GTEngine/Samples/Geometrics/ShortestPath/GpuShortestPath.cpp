// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "GpuShortestPath.h"
#include "Mathematics/GteBitHacks.h"


GpuShortestPath::GpuShortestPath(ProgramFactory& factory,
    std::shared_ptr<Texture2> const& weights, Environment const& env,
    bool& created)
    :
    mSize(static_cast<int>(weights->GetWidth()))
{
    created = false;
    mLogSize = Log2OfPowerOfTwo(mSize);

    mDistance = std::make_shared<Texture2>(DF_R32_FLOAT, mSize, mSize);
    mDistance->SetUsage(Resource::SHADER_OUTPUT);
    memset(mDistance->GetData(), 0, mDistance->GetNumBytes());

    mPrevious = std::make_shared<Texture2>(DF_R32G32_SINT, mSize, mSize);
    mPrevious->SetUsage(Resource::SHADER_OUTPUT);
    mPrevious->SetCopyType(Resource::COPY_STAGING_TO_CPU);

    mSegment = std::make_shared<ConstantBuffer>(3 * sizeof(int), true);

    factory.PushDefines();
    factory.defines.Set("ISIZE", mSize);
    mInitializeDiagToRow = factory.CreateFromFile(
        env.GetPath("InitializeDiagToRow.hlsl"));
    if (!mInitializeDiagToRow)
    {
        return;
    }
    std::shared_ptr<ComputeShader> cshader =
        mInitializeDiagToRow->GetCShader();
    cshader->Set("weights", weights);
    cshader->Set("previous", mPrevious);
    cshader->Set("sum", mDistance);

    mInitializeDiagToCol = factory.CreateFromFile(
        env.GetPath("InitializeDiagToCol.hlsl"));
    if (!mInitializeDiagToCol)
    {
        return;
    }
    cshader = mInitializeDiagToCol->GetCShader();
    cshader->Set("weights", weights);
    cshader->Set("previous", mPrevious);
    cshader->Set("sum", mDistance);

    mPartialSumDiagToRow.resize(mLogSize);
    mPartialSumDiagToCol.resize(mLogSize);
    for (int i = 0; i < mLogSize; ++i)
    {
        factory.defines.Set("LOGN", mLogSize);
        factory.defines.Set("P", i + 1);
        mPartialSumDiagToRow[i] = factory.CreateFromFile(
            env.GetPath("PartialSumsDiagToRow.hlsl"));
        if (!mPartialSumDiagToRow[i])
        {
            return;
        }
        mPartialSumDiagToRow[i]->GetCShader()->Set("sum", mDistance);

        mPartialSumDiagToCol[i] = factory.CreateFromFile(
            env.GetPath("PartialSumsDiagToCol.hlsl"));
        if (!mPartialSumDiagToCol[i])
        {
            return;
        }
        mPartialSumDiagToCol[i]->GetCShader()->Set("sum", mDistance);
    }

    mUpdate = factory.CreateFromFile(env.GetPath("UpdateShader.hlsl"));
    if (!mUpdate)
    {
        return;
    }
    cshader = mUpdate->GetCShader();
    cshader->Set("Segment", mSegment);
    cshader->Set("weights", weights);
    cshader->Set("distance", mDistance);
    cshader->Set("previous", mPrevious);

    factory.PopDefines();
    created = true;
}

void GpuShortestPath::Compute(DX11Engine* engine,
    std::stack<std::pair<int, int>>& path)
{
    // Execute the shaders.
    engine->Execute(mInitializeDiagToRow->GetCShader(), 1, 1, 1);
    for (int i = 0; i < mLogSize; ++i)
    {
        engine->Execute(mPartialSumDiagToRow[i]->GetCShader(), 1, 1, 1);
    }

    engine->Execute(mInitializeDiagToCol->GetCShader(), 1, 1, 1);
    for (int i = 0; i < mLogSize; ++i)
    {
        engine->Execute(mPartialSumDiagToCol[i]->GetCShader(), 1, 1, 1);
    }

    int* segment = mSegment->Get<int>();
    for (int z = 2, numPixels = z - 1; z < mSize; ++z, ++numPixels)
    {
        segment[0] = 1;
        segment[1] = z - 1;
        segment[2] = numPixels;
        engine->Update(mSegment);
        engine->Execute(mUpdate->GetCShader(), 1, 1, 1);
    }

    int const zmax = 2 * (mSize - 1);
    for (int z = mSize, numPixels = zmax - z + 1; z <= zmax; ++z, --numPixels)
    {
        segment[0] = z - (mSize - 1);
        segment[1] = mSize - 1;
        segment[2] = numPixels;
        engine->Update(mSegment);
        engine->Execute(mUpdate->GetCShader(), 1, 1, 1);
    }

    // Read back the path from GPU memory.
    engine->CopyGpuToCpu(mPrevious);
    std::array<int, 2>* location = mPrevious->Get<std::array<int, 2>>();

    // Create the path by starting at (mXSize-1,mYSize-1) and following the
    // previous links.
    int x = mSize - 1, y = mSize - 1;
    while (x != -1 && y != -1)
    {
        path.push(std::make_pair(x, y));
        std::array<int, 2> prev = location[x + mSize*y];
        x = prev[0];
        y = prev[1];
    }
}

