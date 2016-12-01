// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Physics/GteFluid3ComputeDivergence.h>
using namespace gte;


Fluid3ComputeDivergence::~Fluid3ComputeDivergence()
{
}

Fluid3ComputeDivergence::Fluid3ComputeDivergence(ProgramFactory& factory,
    int xSize, int ySize, int zSize, int numXThreads, int numYThreads,
    int numZThreads, std::shared_ptr<ConstantBuffer> const& parameters)
    :
    mNumXGroups(xSize/numXThreads),
    mNumYGroups(ySize/numYThreads),
    mNumZGroups(zSize/numZThreads)
{
    mDivergence = std::make_shared<Texture3>(DF_R32_FLOAT, xSize, ySize,
        zSize);
    mDivergence->SetUsage(Resource::SHADER_OUTPUT);

    int i = factory.GetAPI();
    factory.PushDefines();
    factory.defines.Set("NUM_X_THREADS", numXThreads);
    factory.defines.Set("NUM_Y_THREADS", numYThreads);
    factory.defines.Set("NUM_Z_THREADS", numZThreads);
    mComputeDivergence = factory.CreateFromSource(*msSource[i]);
    if (mComputeDivergence)
    {
        mComputeDivergence->GetCShader()->Set("Parameters", parameters);
        mComputeDivergence->GetCShader()->Set("divergence", mDivergence);
    }

    factory.PopDefines();
}

void Fluid3ComputeDivergence::Execute(
#if defined(GTE_DEV_OPENGL)
    GL4Engine* engine,
#else
    DX11Engine* engine,
#endif
    std::shared_ptr<Texture3> const& state)
{
    std::shared_ptr<ComputeShader> cshader = mComputeDivergence->GetCShader();
    cshader->Set("state", state);
    engine->Execute(cshader, mNumXGroups, mNumYGroups, mNumZGroups);
}


// TODO:  Write this shader.
std::string const Fluid3ComputeDivergence::msGLSLSource = "";

std::string const Fluid3ComputeDivergence::msHLSLSource =
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
"Texture3D<float4> state;\n"
"RWTexture3D<float> divergence;\n"
"\n"
"[numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]\n"
"void CSMain(uint3 c : SV_DispatchThreadID)\n"
"{\n"
"    uint3 dim;\n"
"    state.GetDimensions(dim.x, dim.y, dim.z);\n"
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
"    float3 velocityGradient = float3\n"
"    (\n"
"        state[int3(xp, y, z)].x - state[int3(xm, y, z)].x,\n"
"        state[int3(x, yp, z)].y - state[int3(x, ym, z)].y,\n"
"        state[int3(x, y, zp)].z - state[int3(x, y, zm)].z\n"
"    );\n"
"\n"
"    divergence[c] = dot(halfDivDelta.xyz, velocityGradient);\n"
"}\n";

std::string const* Fluid3ComputeDivergence::msSource[] =
{
    &msGLSLSource,
    &msHLSLSource
};
