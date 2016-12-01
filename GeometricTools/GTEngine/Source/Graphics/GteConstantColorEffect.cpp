// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Graphics/GteConstantColorEffect.h>
using namespace gte;


ConstantColorEffect::ConstantColorEffect(ProgramFactory& factory,
    Vector4<float> const& color)
    :
    mPVWMatrix(nullptr),
    mColor(nullptr)
{
    int i = factory.GetAPI();
    mProgram = factory.CreateFromSources(*msVSSource[i], *msPSSource[i], "");
    if (mProgram)
    {
        mPVWMatrixConstant = std::make_shared<ConstantBuffer>(
            sizeof(Matrix4x4<float>), true);
        mPVWMatrix = mPVWMatrixConstant->Get<Matrix4x4<float>>();
        *mPVWMatrix = Matrix4x4<float>::Identity();

        mColorConstant = std::make_shared<ConstantBuffer>(
            sizeof(Vector4<float>), true);
        mColor = mColorConstant->Get<Vector4<float>>();
        *mColor = color;

        mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
        mProgram->GetVShader()->Set("ConstantColor", mColorConstant);
    }
}


// TODO:  Write these shaders.
std::string const ConstantColorEffect::msGLSLVSSource = "";
std::string const ConstantColorEffect::msGLSLPSSource = "";

std::string const ConstantColorEffect::msHLSLSource =
"cbuffer PVWMatrix\n"
"{\n"
"    float4x4 pvwMatrix;\n"
"};\n"
"\n"
"cbuffer ConstantColor\n"
"{\n"
"    float4 constantColor;\n"
"};\n"
"\n"
"struct VS_INPUT\n"
"{\n"
"    float3 modelPosition : POSITION;\n"
"};\n"
"\n"
"struct VS_OUTPUT\n"
"{\n"
"    float4 vertexColor : COLOR0;\n"
"    float4 clipPosition : SV_POSITION;\n"
"};\n"
"\n"
"VS_OUTPUT VSMain(VS_INPUT input)\n"
"{\n"
"    VS_OUTPUT output;\n"
"#if GTE_USE_MAT_VEC\n"
"    output.clipPosition = mul(pvwMatrix, float4(input.modelPosition, 1.0f));\n"
"#else\n"
"    output.clipPosition = mul(float4(input.modelPosition, 1.0f), pvwMatrix);\n"
"#endif\n"
"    output.vertexColor = constantColor;\n"
"    return output;\n"
"}\n"
"\n"
"struct PS_INPUT\n"
"{\n"
"    float4 vertexColor : COLOR0;\n"
"};\n"
"\n"
"struct PS_OUTPUT\n"
"{\n"
"    float4 pixelColor0 : SV_TARGET0;\n"
"};\n"
"\n"
"PS_OUTPUT PSMain(PS_INPUT input)\n"
"{\n"
"    PS_OUTPUT output;\n"
"    output.pixelColor0 = input.vertexColor;\n"
"    return output;\n"
"}\n";

std::string const* ConstantColorEffect::msVSSource[] =
{
    &msGLSLVSSource,
    &msHLSLSource
};

std::string const* ConstantColorEffect::msPSSource[] =
{
    &msGLSLPSSource,
    &msHLSLSource
};
