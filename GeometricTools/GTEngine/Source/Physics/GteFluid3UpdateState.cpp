// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Physics/GteFluid3UpdateState.h>
using namespace gte;


Fluid3UpdateState::~Fluid3UpdateState()
{
}

Fluid3UpdateState::Fluid3UpdateState(ProgramFactory& factory, int xSize,
    int ySize, int zSize, int numXThreads, int numYThreads, int numZThreads,
    std::shared_ptr<ConstantBuffer> const& parameters)
    :
    mNumXGroups(xSize/numXThreads),
    mNumYGroups(ySize/numYThreads),
    mNumZGroups(zSize/numZThreads)
{
    mUpdateState = std::make_shared<Texture3>(DF_R32G32B32A32_FLOAT, xSize,
        ySize, zSize);
    mUpdateState->SetUsage(Resource::SHADER_OUTPUT);

    mAdvectionSampler = std::make_shared<SamplerState>();
    mAdvectionSampler->filter = SamplerState::MIN_L_MAG_L_MIP_P;
    mAdvectionSampler->mode[0] = SamplerState::CLAMP;
    mAdvectionSampler->mode[1] = SamplerState::CLAMP;
    mAdvectionSampler->mode[2] = SamplerState::CLAMP;

    // Create the shader for generating velocity from vortices.
    int i = factory.GetAPI();
    factory.PushDefines();
    factory.defines.Set("NUM_X_THREADS", numXThreads);
    factory.defines.Set("NUM_Y_THREADS", numYThreads);
    factory.defines.Set("NUM_Z_THREADS", numZThreads);

    mComputeUpdateState = factory.CreateFromSource(*msSource[i]);
    if (mComputeUpdateState)
    {
        std::shared_ptr<ComputeShader> cshader =
            mComputeUpdateState->GetCShader();
        cshader->Set("Parameters", parameters);
        cshader->Set("advectionSampler", mAdvectionSampler);
        cshader->Set("updateState", mUpdateState);
    }

    factory.PopDefines();
}

void Fluid3UpdateState::Execute(
#if defined(GTE_DEV_OPENGL)
    GL4Engine* engine,
#else
    DX11Engine* engine,
#endif
    std::shared_ptr<Texture3> const& source,
    std::shared_ptr<Texture3> const& stateTm1,
    std::shared_ptr<Texture3> const& stateT)
{
    std::shared_ptr<ComputeShader> cshader =
        mComputeUpdateState->GetCShader();
    cshader->Set("source", source);
    cshader->Set("stateTm1", stateTm1);
    cshader->Set("stateT", stateT);
    engine->Execute(cshader, mNumXGroups, mNumYGroups, mNumZGroups);
}


// TODO:  Write this shader.
std::string const Fluid3UpdateState::msGLSLSource = "";

std::string const Fluid3UpdateState::msHLSLSource =
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
"Texture3D<float4> source;\n"
"Texture3D<float4> stateTm1;\n"
"Texture3D<float4> stateT;\n"
"SamplerState advectionSampler;  // trilinear, clamp\n"
"RWTexture3D<float4> updateState;\n"
"\n"
"[numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]\n"
"void CSMain(uint3 c : SV_DispatchThreadID)\n"
"{\n"
"    uint3 dim;\n"
"    stateT.GetDimensions(dim.x, dim.y, dim.z);\n"
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
"    // Sample states at (x,y,z) and immediate neighbors.\n"
"    float4 stateZZZ = stateT[int3(x, y, z)];\n"
"    float4 statePZZ = stateT[int3(xp, y, z)];\n"
"    float4 stateMZZ = stateT[int3(xm, y, z)];\n"
"    float4 stateZPZ = stateT[int3(x, yp, z)];\n"
"    float4 stateZMZ = stateT[int3(x, ym, z)];\n"
"    float4 stateZZP = stateT[int3(x, y, zp)];\n"
"    float4 stateZZM = stateT[int3(x, y, zm)];\n"
"\n"
"    // Sample the source state at (x,y,z).\n"
"    float4 src = source[int3(x, y, z)];\n"
"\n"
"    // Estimate second-order derivatives of state at (x,y,z).\n"
"    float4 stateDXX = statePZZ - 2.0f*stateZZZ + stateMZZ;\n"
"    float4 stateDYY = stateZPZ - 2.0f*stateZZZ + stateZMZ;\n"
"    float4 stateDZZ = stateZZP - 2.0f*stateZZZ + stateZZM;\n"
"\n"
"    // Compute advection.\n"
"    float3 tcd = spaceDelta.xyz*(c.xyz - timeDelta.xyz*stateZZZ.xyz + 0.5f);\n"
"    float4 advection = stateTm1.SampleLevel(advectionSampler, tcd, 0.0f);\n"
"\n"
"    // Update the state.\n"
"    updateState[c.xyz] = advection +\n"
"        (viscosityX*stateDXX + viscosityY*stateDYY + +viscosityZ*stateDZZ +\n"
"        timeDelta.w*src);\n"
"}\n";

std::string const* Fluid3UpdateState::msSource[] =
{
    &msGLSLSource,
    &msHLSLSource
};
