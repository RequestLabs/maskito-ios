// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEngine.h>
using namespace gte;


void TestAppendConsumeBuffer()
{
    Environment env;
    std::string gtpath = env.GetVariable("GTE_PATH");
    if (gtpath == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return;
    }
    env.Insert(gtpath + "/Samples/Basics/AppendConsumeBuffers/Shaders/");

    // Create a shader that trims the initial set by half, consuming only
    // the even-indexed particles.
    std::string path = env.GetPath("AppendConsume.hlsl");
    if (path == "")
    {
        LogError("Cannot find AppendConsume.hlsl.");
        return;
    }

    HLSLProgramFactory factory;
    std::shared_ptr<ComputeProgram> cprogram = factory.CreateFromFile(path);
    if (!cprogram)
    {
        return;
    }

    DX11Engine engine(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, D3D_FEATURE_LEVEL_11_0);

    // Create 32 particles, stored in currentState to be "consumed".
    struct Particle
    {
        int location[2];
    };
    int const numInputs = 32;
    std::shared_ptr<StructuredBuffer> currentState =
        std::make_shared<StructuredBuffer>(numInputs, sizeof(Particle));
    currentState->MakeAppendConsume();

    Particle* particle = currentState->Get<Particle>();
    for (int i = 0; i < numInputs; ++i)
    {
        particle[i].location[0] = i;
        particle[i].location[1] = rand();
    }

    // The next set of particles is created from the initial set.
    std::shared_ptr<StructuredBuffer> nextState =
        std::make_shared<StructuredBuffer>(numInputs, sizeof(Particle));
    nextState->MakeAppendConsume();
    nextState->SetCopyType(Resource::COPY_STAGING_TO_CPU);

    // Start with an empty buffer to which particles are "appended".
    nextState->SetNumActiveElements(0);

    std::shared_ptr<ComputeShader> cshader = cprogram->GetCShader();
    cshader->Set("currentState", currentState);
    cshader->Set("nextState", nextState);

    // Compute the next set of particles.
    engine.Execute(cshader, 1, 1, 1);

    // Read back the data from the GPU to test whether we really have
    // consumed half the initial set.
    engine.CopyGpuToCpu(nextState);
    int numNextState = nextState->GetNumActiveElements();
    if (numNextState != numInputs / 2)
    {
        LogError("Invalid number of active elements.");
    }

    // Verify that the data was consumed properly.
    Particle* nextParticle = nextState->Get<Particle>();
    for (int i = 0; i < numInputs / 2; ++i)
    {
        int j = nextParticle[i].location[0];
        if ((j & 1) != 0)
        {
            LogError("Invalid index.");
        }

        if (nextParticle[i].location[1] != particle[j].location[1])
        {
            LogError("Invalid value.");
        }
    }

    // Release the append-consume buffers and compute shader so that the
    // engine does not report nonempty bridge maps on destruction.
    currentState = nullptr;
    nextState = nullptr;
    cshader = nullptr;
    cprogram = nullptr;
}

int main(int, char const*[])
{
#if defined(_DEBUG)
    LogReporter reporter(
        "LogReport.txt",
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL);
#endif

    TestAppendConsumeBuffer();
    return 0;
}

