// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Physics/GteFluid2EnforceStateBoundary.h>
using namespace gte;


Fluid2EnforceStateBoundary::~Fluid2EnforceStateBoundary()
{
}

Fluid2EnforceStateBoundary::Fluid2EnforceStateBoundary(
    ProgramFactory& factory, int xSize, int ySize, int numXThreads,
    int numYThreads)
    :
    mNumXGroups(xSize/numXThreads),
    mNumYGroups(ySize/numYThreads)
{
    mXMin = std::make_shared<Texture1>(DF_R32_FLOAT, ySize);
    mXMin->SetUsage(Resource::SHADER_OUTPUT);
    mXMax = std::make_shared<Texture1>(DF_R32_FLOAT, ySize);
    mXMax->SetUsage(Resource::SHADER_OUTPUT);
    mYMin = std::make_shared<Texture1>(DF_R32_FLOAT, xSize);
    mYMin->SetUsage(Resource::SHADER_OUTPUT);
    mYMax = std::make_shared<Texture1>(DF_R32_FLOAT, xSize);
    mYMax->SetUsage(Resource::SHADER_OUTPUT);

    int i = factory.GetAPI();
    factory.PushDefines();
    factory.defines.Set("USE_COPY_X_EDGE", 1);
    factory.defines.Set("NUM_Y_THREADS", numYThreads);
    mCopyXEdge = factory.CreateFromSource(*msSource[i]);
    if (mCopyXEdge)
    {
        mCopyXEdge->GetCShader()->Set("xMin", mXMin);
        mCopyXEdge->GetCShader()->Set("xMax", mXMax);
    }

    factory.defines.Clear();
    factory.defines.Set("USE_WRITE_X_EDGE", 1);
    factory.defines.Set("NUM_Y_THREADS", numYThreads);
    mWriteXEdge = factory.CreateFromSource(*msSource[i]);
    if (mWriteXEdge)
    {
        mWriteXEdge->GetCShader()->Set("xMin", mXMin);
        mWriteXEdge->GetCShader()->Set("xMax", mXMax);
    }

    factory.defines.Clear();
    factory.defines.Set("USE_COPY_Y_EDGE", 1);
    factory.defines.Set("NUM_X_THREADS", numXThreads);
    mCopyYEdge = factory.CreateFromSource(*msSource[i]);
    if (mCopyYEdge)
    {
        mCopyYEdge->GetCShader()->Set("yMin", mYMin);
        mCopyYEdge->GetCShader()->Set("yMax", mYMax);
    }

    factory.defines.Clear();
    factory.defines.Set("USE_WRITE_Y_EDGE", 1);
    factory.defines.Set("NUM_X_THREADS", numXThreads);
    mWriteYEdge = factory.CreateFromSource(*msSource[i]);
    if (mWriteYEdge)
    {
        mWriteYEdge->GetCShader()->Set("yMin", mYMin);
        mWriteYEdge->GetCShader()->Set("yMax", mYMax);
    }

    factory.PopDefines();
}

void Fluid2EnforceStateBoundary::Execute(
#if defined(GTE_DEV_OPENGL)
    GL4Engine* engine,
#else
    DX11Engine* engine,
#endif
    std::shared_ptr<Texture2> const& state)
{
    // in: state
    // out: mXMin, mXMax
    mCopyXEdge->GetCShader()->Set("state", state);
    engine->Execute(mCopyXEdge->GetCShader(), 1, mNumYGroups, 1);

    // in: mXMin, mXMax
    // out: state
    mWriteXEdge->GetCShader()->Set("state", state);
    engine->Execute(mWriteXEdge->GetCShader(), 1, mNumYGroups, 1);

    // in: state
    // out: mYMin, mYMax
    mCopyYEdge->GetCShader()->Set("state", state);
    engine->Execute(mCopyYEdge->GetCShader(), mNumXGroups, 1, 1);

    // in: mYMin, mYMax
    // out: state
    mWriteYEdge->GetCShader()->Set("state", state);
    engine->Execute(mWriteYEdge->GetCShader(), mNumXGroups, 1, 1);
}


// TODO:  Write this shader.
std::string const Fluid2EnforceStateBoundary::msGLSLSource = "";

std::string const Fluid2EnforceStateBoundary::msHLSLSource =
"#if USE_COPY_X_EDGE\n"
"Texture2D<float4> state;\n"
"RWTexture1D<float> xMin;\n"
"RWTexture1D<float> xMax;\n"
"\n"
"[numthreads(1, NUM_Y_THREADS, 1)]\n"
"void CSMain(uint2 c : SV_DispatchThreadID)\n"
"{\n"
"    uint2 dim;\n"
"    state.GetDimensions(dim.x, dim.y);\n"
"    xMin[c.y] = state[uint2(1, c.y)].y;\n"
"    xMax[c.y] = state[uint2(dim.x - 2, c.y)].y;\n"
"}\n"
"#endif\n"
"\n"
"#if USE_WRITE_X_EDGE\n"
"Texture1D<float> xMin;\n"
"Texture1D<float> xMax;\n"
"RWTexture2D<float4> state;\n"
"\n"
"[numthreads(1, NUM_Y_THREADS, 1)]\n"
"void CSMain(uint2 c : SV_DispatchThreadID)\n"
"{\n"
"    uint2 dim;\n"
"    state.GetDimensions(dim.x, dim.y);\n"
"    state[uint2(0, c.y)] = float4(0.0f, xMin[c.y], 0.0f, 0.0f);\n"
"    state[uint2(dim.x - 1, c.y)] = float4(0.0f, xMax[c.y], 0.0f, 0.0f);\n"
"}\n"
"#endif\n"
"\n"
"#if USE_COPY_Y_EDGE\n"
"Texture2D<float4> state;\n"
"RWTexture1D<float> yMin;\n"
"RWTexture1D<float> yMax;\n"
"\n"
"[numthreads(NUM_X_THREADS, 1, 1)]\n"
"void CSMain(uint2 c : SV_DispatchThreadID)\n"
"{\n"
"    uint2 dim;\n"
"    state.GetDimensions(dim.x, dim.y);\n"
"    yMin[c.x] = state[uint2(c.x, 1)].x;\n"
"    yMax[c.x] = state[uint2(c.x, dim.y - 2)].x;\n"
"}\n"
"#endif\n"
"\n"
"#if USE_WRITE_Y_EDGE\n"
"Texture1D<float> yMin;\n"
"Texture1D<float> yMax;\n"
"RWTexture2D<float4> state;\n"
"\n"
"[numthreads(NUM_X_THREADS, 1, 1)]\n"
"void CSMain(uint2 c : SV_DispatchThreadID)\n"
"{\n"
"    uint2 dim;\n"
"    state.GetDimensions(dim.x, dim.y);\n"
"    state[uint2(c.x, 0)] = float4(yMin[c.x], 0.0f, 0.0f, 0.0f);\n"
"    state[uint2(c.x, dim.y - 1)] = float4(yMax[c.x], 0.0f, 0.0f, 0.0f);\n"
"}\n"
"#endif\n";

std::string const* Fluid2EnforceStateBoundary::msSource[] =
{
    &msGLSLSource,
    &msHLSLSource
};
