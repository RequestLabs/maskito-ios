// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEngine.h>
#include <iostream>
#include <thread>
using namespace gte;


float MyFunction(float z)
{
    return (z - 1.1f)*(z + 2.2f);
}

void FindRootsCPU(std::set<float>& roots)
{
    unsigned int const supTrailing = (1 << 23);
    for (unsigned int trailing = 0; trailing < supTrailing; ++trailing)
    {
        for (unsigned int biased = 0; biased < 255; ++biased)
        {
            unsigned int exponent = (biased << 23);
            unsigned int encoding0 = exponent | trailing;
            unsigned int encoding1 = encoding0 + 1;
            float z0 = *(float*)&encoding0;
            float z1 = *(float*)&encoding1;

            float f0 = MyFunction(z0);
            float f1 = MyFunction(z1);
            if (f0*f1 <= 0.0f)
            {
                roots.insert(std::abs(f0) <= std::abs(f1) ? z0 : z1);
            }

            z0 = -z0;
            z1 = -z1;
            f0 = MyFunction(z0);
            f1 = MyFunction(z1);
            if (f0*f1 <= 0.0f)
            {
                roots.insert(std::abs(f0) <= std::abs(f1) ? z0 : z1);
            }
        }
    }
}

void FindRootsGPU(std::set<float>& roots)
{
    DX11Engine engine(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        D3D_FEATURE_LEVEL_11_0);

    Environment env;
    std::string path = env.GetVariable("GTE_PATH");
    if (path == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return;
    }
    env.Insert(path + "/Samples/Mathematics/RootFinding/Shaders/");
    path = env.GetPath("RootFinder.hlsl");
    if (path == "")
    {
        LogError("Cannot find file. ");
        return;
    }

    HLSLProgramFactory factory;
    factory.defines.Set("FUNCTION_BODY", "(z - 1.1f)*(z + 2.2f)");
    std::shared_ptr<ComputeProgram> cprogram = factory.CreateFromFile(path);
    if (!cprogram)
    {
        return;
    }

    std::shared_ptr<StructuredBuffer> acBuffer(new StructuredBuffer(1024,
        sizeof(Vector4<float>)));
    acBuffer->MakeAppendConsume();
    acBuffer->SetCopyType(Resource::COPY_STAGING_TO_CPU);
    acBuffer->SetNumActiveElements(0);

    cprogram->GetCShader()->Set("rootBounds", acBuffer);

    engine.Execute(cprogram->GetCShader(), 512, 256, 1);

    engine.CopyGpuToCpu(acBuffer);
    int numActive = acBuffer->GetNumActiveElements();
    Vector4<float>* rootBounds = acBuffer->Get<Vector4<float>>();
    for (int i = 0; i < numActive; ++i)
    {
        Vector4<float> const& rb = rootBounds[i];
        if (std::abs(rb[1]) <= std::abs(rb[3]))
        {
            roots.insert(rb[0]);
        }
        else
        {
            roots.insert(rb[2]);
        }
    }

    acBuffer = nullptr;
    cprogram = nullptr;
}

void FindSubRootsCPU(unsigned int tmin, unsigned int tsup,
    std::set<float>& roots)
{
    for (unsigned int trailing = tmin; trailing < tsup; ++trailing)
    {
        for (unsigned int biased = 0; biased < 255; ++biased)
        {
            unsigned int exponent = (biased << 23);
            unsigned int encoding0 = exponent | trailing;
            unsigned int encoding1 = encoding0 + 1;
            float z0 = *(float*)&encoding0;
            float z1 = *(float*)&encoding1;

            float f0 = MyFunction(z0);
            float f1 = MyFunction(z1);
            if (f0*f1 <= 0.0f)
            {
                roots.insert(std::abs(f0) <= std::abs(f1) ? z0 : z1);
            }

            z0 = -z0;
            z1 = -z1;
            f0 = MyFunction(z0);
            f1 = MyFunction(z1);
            if (f0*f1 <= 0.0f)
            {
                roots.insert(std::abs(f0) <= std::abs(f1) ? z0 : z1);
            }
        }
    }
}

void FindRootsCPUMultithreaded(std::set<float>& roots)
{
    int const numThreads = 16;
    unsigned int const supTrailing = (1 << 23);
    std::set<float> subRoots[numThreads];

    std::thread process[numThreads];
    for (int t = 0; t < numThreads; ++t)
    {
        unsigned int tmin = t * supTrailing / numThreads;
        unsigned int tsup = (t + 1) * supTrailing / numThreads;
        auto rootFinder = std::bind(FindSubRootsCPU, tmin, tsup,
            std::ref(subRoots[t]));

        process[t] = std::thread([&rootFinder](){ rootFinder(); });
    }

    for (int t = 0; t < numThreads; ++t)
    {
        process[t].join();
    }

    for (int t = 0; t < numThreads; ++t)
    {
        for (auto const& z : subRoots[t])
        {
            roots.insert(z);
        }
    }
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

    std::set<float> rootsCPU, rootsCPUMultithreaded, rootsGPU;

    Timer timer;
    double start, final, delta;
    std::string message;

    start = timer.GetSeconds();
    FindRootsCPU(rootsCPU);
    final = timer.GetSeconds();
    delta = final - start;
    message = "CPU: " + std::to_string(delta) + "\n";
    std::cout << message;

    start = timer.GetSeconds();
    FindRootsCPUMultithreaded(rootsCPUMultithreaded);
    final = timer.GetSeconds();
    delta = final - start;
    message = "CPU multithreaded: " + std::to_string(delta) + "\n";
    std::cout << message;

    start = timer.GetSeconds();
    FindRootsGPU(rootsGPU);
    final = timer.GetSeconds();
    delta = final - start;
    message = "GPU: " + std::to_string(delta) + "\n";
    std::cout << message;
    return 0;
}

