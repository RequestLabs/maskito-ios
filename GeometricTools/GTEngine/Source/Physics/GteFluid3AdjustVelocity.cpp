// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Physics/GteFluid3AdjustVelocity.h>
using namespace gte;


Fluid3AdjustVelocity::~Fluid3AdjustVelocity()
{
}

Fluid3AdjustVelocity::Fluid3AdjustVelocity(ProgramFactory& factory, int xSize,
    int ySize, int zSize, int numXThreads, int numYThreads, int numZThreads,
    std::shared_ptr<ConstantBuffer> const& parameters)
    :
    mNumXGroups(xSize/numXThreads),
    mNumYGroups(ySize/numYThreads),
    mNumZGroups(zSize/numZThreads)
{
    int i = factory.GetAPI();
    factory.PushDefines();
    factory.defines.Set("NUM_X_THREADS", numXThreads);
    factory.defines.Set("NUM_Y_THREADS", numYThreads);
    factory.defines.Set("NUM_Z_THREADS", numZThreads);

    mAdjustVelocity = factory.CreateFromSource(*msSource[i]);
    if (mAdjustVelocity)
    {
        mAdjustVelocity->GetCShader()->Set("Parameters", parameters);
    }

    factory.PopDefines();
}

void Fluid3AdjustVelocity::Execute(
#if defined(GTE_DEV_OPENGL)
    GL4Engine* engine,
#else
    DX11Engine* engine,
#endif
    std::shared_ptr<Texture3> const& inState,
    std::shared_ptr<Texture3> const& poisson,
    std::shared_ptr<Texture3> const& outState)
{
    std::shared_ptr<ComputeShader> cshader = mAdjustVelocity->GetCShader();
    cshader->Set("inState", inState);
    cshader->Set("poisson", poisson);
    cshader->Set("outState", outState);
    engine->Execute(cshader, mNumXGroups, mNumYGroups, mNumZGroups);
}


// TODO:  Write this shader.
std::string const Fluid3AdjustVelocity::msGLSLSource = "";

std::string const Fluid3AdjustVelocity::msHLSLSource =
"cbuffer Parameters\n"
"{\n"
"    float4 spaceDelta;    // (dx, dy, dz, 0)\n"
"    float4 halfDivDelta;  // (0.5/dx, 0.5/dy, 0.5/dz, 0)\n"
"    float4 timeDelta;     // (dt/dx, dt/dy, dt/dz, dt)\n"
"    float4 viscosityX;    // (velVX, velVX, velVX, denVX)\n"
"    float4 viscosityY;    // (velVX, velVY, velVY, denVY)\n"
"    float4 viscosityZ;    // (velVZ, velVZ, velVZ, denVZ)\n"
"    float4 epsilon;       // (epsilonX, epsilonY, epsilonZ, epsilon0)\n"
"};\n"
"\n"
"Texture3D<float4> inState;\n"
"Texture3D<float> poisson;\n"
"RWTexture3D<float4> outState;\n"
"\n"
"[numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]\n"
"void CSMain(uint3 c : SV_DispatchThreadID)\n"
"{\n"
"    uint3 dim;\n"
"    inState.GetDimensions(dim.x, dim.y, dim.z);\n"
"\n"
"    int x = int(c.x);\n"
"    int y = int(c.y);\n"
"    int z = int(c.z);\n"
"    int xm = max(x - 1, 0);\n"
"    int xp = min(x + 1, dim.x - 1);\n"
"    int ym = max(y - 1, 0);\n"
"    int yp = min(y + 1, dim.y - 1);\n"
"    int zm = max(z - 1, 0);\n"
"    int zp = min(z + 1, dim.z - 1);\n"
"\n"
"    // Sample the state at (x,y,z).\n"
"    float4 state = inState[c.xyz];\n"
"\n"
"    // Sample Poisson values at immediate neighbors of (x,y,z).\n"
"    float poisPZZ = poisson[int3(xp, y, z)];\n"
"    float poisMZZ = poisson[int3(xm, y, z)];\n"
"    float poisZPZ = poisson[int3(x, yp, z)];\n"
"    float poisZMZ = poisson[int3(x, ym, z)];\n"
"    float poisZZP = poisson[int3(x, y, zp)];\n"
"    float poisZZM = poisson[int3(x, y, zm)];\n"
"\n"
"    float4 diff = float4(poisPZZ - poisMZZ, poisZPZ - poisZMZ,\n"
"        poisZZP - poisZZM, 0.0f);\n"
"    outState[c.xyz] = state + halfDivDelta*diff;\n"
"}\n";

std::string const* Fluid3AdjustVelocity::msSource[] =
{
    &msGLSLSource,
    &msHLSLSource
};
