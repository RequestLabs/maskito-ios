// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEngine.h>
using namespace gte;


void TestRawBuffer()
{
    DX11Engine engine(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, D3D_FEATURE_LEVEL_11_0);

    Environment env;
    std::string path = env.GetVariable("GTE_PATH");
    if (path == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return;
    }

    path += "/Samples/Basics/RawBuffers/Shaders/";
    path += "RawBuffers.hlsl";
    HLSLProgramFactory factory;
    std::shared_ptr<ComputeProgram> cprogram = factory.CreateFromFile(path);
    if (!cprogram)
    {
        return;
    }

    std::shared_ptr<RawBuffer> inputBuffer = std::make_shared<RawBuffer>(4);
    char* input = inputBuffer->GetData();
    *(unsigned char*)&input[0] = 'a';
    *(double*)&input[1] = (double)GTE_C_PI;
    *(float*)&input[9] = (float)GTE_C_PI;
    *(short*)&input[13] = -1;
    *(unsigned char*)&input[15] = 'b';

    std::shared_ptr<RawBuffer> outputBuffer = std::make_shared<RawBuffer>(4);
    outputBuffer->SetUsage(Resource::SHADER_OUTPUT);
    outputBuffer->SetCopyType(Resource::COPY_STAGING_TO_CPU);

    std::shared_ptr<ComputeShader> cshader = cprogram->GetCShader();
    cshader->Set("input", inputBuffer);
    cshader->Set("output", outputBuffer);

    engine.Execute(cshader, 1, 1, 1);

    engine.CopyGpuToCpu(outputBuffer);
    char* output = outputBuffer->GetData();
    double pid;
    float pif;
    short minusOne;
    unsigned char a, b;
    pid = *(double*)&output[0];         // 3.1415926535897931
    pif = *(float*)&output[8];          // 3.14159274
    minusOne = *(short*)&output[12];    // -1
    a = *(unsigned char*)&output[14];   // 'a'
    b = *(unsigned char*)&output[15];   // 'b'

    outputBuffer = nullptr;
    inputBuffer = nullptr;
    cshader = nullptr;
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

    TestRawBuffer();
    return 0;
}

