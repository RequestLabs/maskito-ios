// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Physics/GteFluid2AdjustVelocity.h>
using namespace gte;


Fluid2AdjustVelocity::~Fluid2AdjustVelocity()
{
}

Fluid2AdjustVelocity::Fluid2AdjustVelocity(ProgramFactory& factory,
    int xSize, int ySize, int numXThreads, int numYThreads,
    std::shared_ptr<ConstantBuffer> const& parameters)
    :
    mNumXGroups(xSize/numXThreads),
    mNumYGroups(ySize/numYThreads)
{
    int i = factory.GetAPI();
    factory.PushDefines();
    factory.defines.Set("NUM_X_THREADS", numXThreads);
    factory.defines.Set("NUM_Y_THREADS", numYThreads);

    mAdjustVelocity = factory.CreateFromSource(*msSource[i]);
    if (mAdjustVelocity)
    {
        mAdjustVelocity->GetCShader()->Set("Parameters", parameters);
    }

    factory.PopDefines();
}

void Fluid2AdjustVelocity::Execute(
#if defined(GTE_DEV_OPENGL)
    GL4Engine* engine,
#else
    DX11Engine* engine,
#endif
    std::shared_ptr<Texture2> const& inState,
    std::shared_ptr<Texture2> const& poisson,
    std::shared_ptr<Texture2> const& outState)

{
    std::shared_ptr<ComputeShader> cshader = mAdjustVelocity->GetCShader();
    cshader->Set("inState", inState);
    cshader->Set("poisson", poisson);
    cshader->Set("outState", outState);
    engine->Execute(cshader, mNumXGroups, mNumYGroups, 1);
}


// TODO:  Write this shader.
std::string const Fluid2AdjustVelocity::msGLSLSource = "";

std::string const Fluid2AdjustVelocity::msHLSLSource =
"cbuffer Parameters\n"
"{\n"
"    float4 spaceDelta;    // (dx, dy, 0, 0)\n"
"    float4 halfDivDelta;  // (0.5/dx, 0.5/dy, 0, 0)\n"
"    float4 timeDelta;     // (dt/dx, dt/dy, 0, dt)\n"
"    float4 viscosityX;    // (velVX, velVX, 0, denVX)\n"
"    float4 viscosityY;    // (velVX, velVY, 0, denVY)\n"
"    float4 epsilon;       // (epsilonX, epsilonY, 0, epsilon0)\n"
"};\n"
"\n"
"Texture2D<float4> inState;\n"
"Texture2D<float> poisson;\n"
"RWTexture2D<float4> outState;\n"
"\n"
"[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]\n"
"void CSMain(uint2 c : SV_DispatchThreadID)\n"
"{\n"
"    uint2 dim;\n"
"    inState.GetDimensions(dim.x, dim.y);\n"
"\n"
"    int x = int(c.x);\n"
"    int y = int(c.y);\n"
"    int xm = max(x - 1, 0);\n"
"    int xp = min(x + 1, dim.x - 1);\n"
"    int ym = max(y - 1, 0);\n"
"    int yp = min(y + 1, dim.y - 1);\n"
"\n"
"    // Sample the state at (x,y).\n"
"    float4 state = inState[c];\n"
"\n"
"    // Sample Poisson values at immediate neighbors of (x,y).\n"
"    float poisPZ = poisson[int2(xp, y)];\n"
"    float poisMZ = poisson[int2(xm, y)];\n"
"    float poisZP = poisson[int2(x, yp)];\n"
"    float poisZM = poisson[int2(x, ym)];\n"
"\n"
"    float4 diff = float4(poisPZ - poisMZ, poisZP - poisZM, 0.0f, 0.0f);\n"
"    outState[c] = state + halfDivDelta*diff;\n"
"}\n";

std::string const* Fluid2AdjustVelocity::msSource[] =
{
    &msGLSLSource,
    &msHLSLSource
};
