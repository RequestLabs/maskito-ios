// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <Graphics/DX11/GteHLSLFactory.h>
#include <fstream>
using namespace gte;

unsigned int const gCompileFlags =
    D3DCOMPILE_ENABLE_STRICTNESS |
    D3DCOMPILE_IEEE_STRICTNESS |
    D3DCOMPILE_PACK_MATRIX_ROW_MAJOR |
    D3DCOMPILE_OPTIMIZATION_LEVEL3;


void ReflectVertexColoring()
{
    HLSLShader vshader = HLSLShaderFactory::CreateFromFile(
        "VertexColoring.hlsl",
        "VSMain",
        "vs_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (vshader.IsValid())
    {
        std::ofstream vsout("VertexColoring.vsreflect.txt");
        if (vsout)
        {
            vshader.Print(vsout);
            vsout.close();
        }
    }

    HLSLShader pshader = HLSLShaderFactory::CreateFromFile(
        "VertexColoring.hlsl",
        "PSMain",
        "ps_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (pshader.IsValid())
    {
        std::ofstream psout("VertexColoring.psreflect.txt");
        if (psout)
        {
            pshader.Print(psout);
            psout.close();
        }
    }
}

void ReflectTexturing()
{
    HLSLShader vshader = HLSLShaderFactory::CreateFromFile(
        "Texturing.hlsl",
        "VSMain",
        "vs_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (vshader.IsValid())
    {
        std::ofstream vsout("Texturing.vsreflect.txt");
        if (vsout)
        {
            vshader.Print(vsout);
            vsout.close();
        }
    }

    HLSLShader pshader = HLSLShaderFactory::CreateFromFile(
        "Texturing.hlsl",
        "PSMain",
        "ps_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (pshader.IsValid())
    {
        std::ofstream psout("Texturing.psreflect.txt");
        if (psout)
        {
            pshader.Print(psout);
            psout.close();
        }
    }
}

void ReflectBillboards()
{
    HLSLShader vshader = HLSLShaderFactory::CreateFromFile(
        "Billboards.hlsl",
        "VSMain",
        "vs_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (vshader.IsValid())
    {
        std::ofstream vsout("Billboards.vsreflect.txt");
        if (vsout)
        {
            vshader.Print(vsout);
            vsout.close();
        }
    }

    HLSLShader gshader = HLSLShaderFactory::CreateFromFile(
        "Billboards.hlsl",
        "GSMain",
        "gs_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (gshader.IsValid())
    {
        std::ofstream gsout("Billboards.gsreflect.txt");
        if (gsout)
        {
            gshader.Print(gsout);
            gsout.close();
        }
    }

    HLSLShader pshader = HLSLShaderFactory::CreateFromFile(
        "Billboards.hlsl",
        "PSMain",
        "ps_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (pshader.IsValid())
    {
        std::ofstream psout("Billboards.psreflect.txt");
        if (psout)
        {
            pshader.Print(psout);
            psout.close();
        }
    }
}

void ReflectNestedStruct()
{
    HLSLShader cshader = HLSLShaderFactory::CreateFromFile(
        "NestedStruct.hlsl",
        "CSMain",
        "cs_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (cshader.IsValid())
    {
        std::ofstream csout("NestedStruct.csreflect.txt");
        if (csout)
        {
            cshader.Print(csout);
            csout.close();
        }
    }
}

void ReflectTextureArrays()
{
    HLSLShader vshader = HLSLShaderFactory::CreateFromFile(
        "TextureArrays.hlsl",
        "VSMain",
        "vs_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (vshader.IsValid())
    {
        std::ofstream vsout("TextureArrays.vsreflect.txt");
        if (vsout)
        {
            vshader.Print(vsout);
            vsout.close();
        }
    }

    HLSLShader pshader = HLSLShaderFactory::CreateFromFile(
        "TextureArrays.hlsl",
        "PSMain",
        "ps_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (pshader.IsValid())
    {
        std::ofstream psout("TextureArrays.psreflect.txt");
        if (psout)
        {
            pshader.Print(psout);
            psout.close();
        }
    }
}

void ReflectSimpleBuffers()
{
    HLSLShader cshader = HLSLShaderFactory::CreateFromFile(
        "SimpleBuffers.hlsl",
        "CSMain",
        "cs_5_0",
        ProgramDefines(),
        gCompileFlags);

    if (cshader.IsValid())
    {
        std::ofstream csout("SimpleBuffers.csreflect.txt");
        if (csout)
        {
            cshader.Print(csout);
            csout.close();
        }
    }
}

int main(int, char const*[])
{
    ReflectVertexColoring();
    ReflectTexturing();
    ReflectBillboards();
    ReflectNestedStruct();
    ReflectTextureArrays();
    ReflectSimpleBuffers();
    return 0;
}

