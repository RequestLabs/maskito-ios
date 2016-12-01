// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Physics/GteFluid2ComputeDivergence.h>
using namespace gte;


Fluid2ComputeDivergence::~Fluid2ComputeDivergence()
{
}

Fluid2ComputeDivergence::Fluid2ComputeDivergence(ProgramFactory& factory,
    int xSize, int ySize, int numXThreads, int numYThreads,
    std::shared_ptr<ConstantBuffer> const& parameters)
    :
    mNumXGroups(xSize/numXThreads),
    mNumYGroups(ySize/numYThreads)
{
    mDivergence = std::make_shared<Texture2>(DF_R32_FLOAT, xSize, ySize);
    mDivergence->SetUsage(Resource::SHADER_OUTPUT);

    int i = factory.GetAPI();
    factory.PushDefines();
    factory.defines.Set("NUM_X_THREADS", numXThreads);
    factory.defines.Set("NUM_Y_THREADS", numYThreads);
    mComputeDivergence = factory.CreateFromSource(*msSource[i]);
    if (mComputeDivergence)
    {
        mComputeDivergence->GetCShader()->Set("Parameters", parameters);
        mComputeDivergence->GetCShader()->Set("divergence", mDivergence);
    }

    factory.PopDefines();
}

void Fluid2ComputeDivergence::Execute(
#if defined(GTE_DEV_OPENGL)
    GL4Engine* engine,
#else
    DX11Engine* engine,
#endif
    std::shared_ptr<Texture2> const& state)
{
    std::shared_ptr<ComputeShader> cshader = mComputeDivergence->GetCShader();
    cshader->Set("state", state);
    engine->Execute(cshader, mNumXGroups, mNumYGroups, 1);
}


// TODO:  Write this shader.
std::string const Fluid2ComputeDivergence::msGLSLSource = "";

std::string const Fluid2ComputeDivergence::msHLSLSource =
"cbuffer Parameters\n"
"{\n"
"    float4 spaceDelta;    // (dx, dy, 0, 0)\n"
"    float4 halfDivDelta;  // (0.5/dx, 0.5/dy, 0, 0)\n"
"    float4 timeDelta;     // (dt, dt/dx, dt/dy, 0)\n"
"    float4 viscosityX;    // (velVX, velVX, 0, denVX)\n"
"    float4 viscosityY;    // (velVX, velVY, 0, denVY)\n"
"    float4 epsilon;       // (epsilonX, epsilonY, 0, epsilon0)\n"
"};\n"
"\n"
"Texture2D<float4> state;\n"
"RWTexture2D<float> divergence;\n"
"\n"
"[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]\n"
"void CSMain(uint2 c : SV_DispatchThreadID)\n"
"{\n"
"    uint2 dim;\n"
"    state.GetDimensions(dim.x, dim.y);\n"
"\n"
"    int x = int(c.x);\n"
"    int y = int(c.y);\n"
"    int xm = max(x - 1, 0);\n"
"    int xp = min(x + 1, dim.x - 1);\n"
"    int ym = max(y - 1, 0);\n"
"    int yp = min(y + 1, dim.y - 1);\n"
"\n"
"    float2 velocityGradient = float2(\n"
"        state[int2(xp, y)].x - state[int2(xm, y)].x,\n"
"        state[int2(x, yp)].y - state[int2(x, ym)].y\n"
"    );\n"
"\n"
"    divergence[c] = dot(halfDivDelta.xy, velocityGradient);\n"
"}\n";

std::string const* Fluid2ComputeDivergence::msSource[] =
{
    &msGLSLSource,
    &msHLSLSource
};
