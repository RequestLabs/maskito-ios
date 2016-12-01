// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Graphics/GteAmbientLightEffect.h>
using namespace gte;

AmbientLightEffect::AmbientLightEffect(ProgramFactory& factory, BufferUpdater const& updater,
    std::shared_ptr<Material> const& material, std::shared_ptr<Lighting> const& lighting)
    :
    LightingEffect(factory, updater, msVSSource, msPSSource, material, lighting, nullptr)
{
    mMaterialConstant = std::make_shared<ConstantBuffer>(sizeof(InternalMaterial), true);
    UpdateMaterialConstant();
    mProgram->GetVShader()->Set("Material", mMaterialConstant);

    mLightingConstant = std::make_shared<ConstantBuffer>(sizeof(InternalLighting), true);
    UpdateLightingConstant();
    mProgram->GetVShader()->Set("Lighting", mLightingConstant);
}

void AmbientLightEffect::UpdateMaterialConstant()
{
    InternalMaterial* internalMaterial = mMaterialConstant->Get<InternalMaterial>();
    internalMaterial->emissive = mMaterial->emissive;
    internalMaterial->ambient = mMaterial->ambient;
    LightingEffect::UpdateMaterialConstant();
}

void AmbientLightEffect::UpdateLightingConstant()
{
    InternalLighting* internalLighting = mLightingConstant->Get<InternalLighting>();
    internalLighting->ambient = mLighting->ambient;
    internalLighting->attenuation = mLighting->attenuation;
    LightingEffect::UpdateLightingConstant();
}

// TODO:  Write these shaders.
std::string const AmbientLightEffect::msGLSLVSSource = "";
std::string const AmbientLightEffect::msGLSLPSSource = "";

std::string const AmbientLightEffect::msHLSLSource =
"cbuffer PVWMatrix\n"
"{\n"
"    float4x4 pvwMatrix;\n"
"};\n"
"\n"
"cbuffer Material\n"
"{\n"
"    float4 materialEmissive;\n"
"    float4 materialAmbient;\n"
"};\n"
"\n"
"cbuffer Light\n"
"{\n"
"    float4 lightingAmbient;\n"
"    float4 lightingAttenuation;\n"
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
"\n"
"    float3 ambient = lightingAttenuation.w * lightingAmbient.rgb;\n"
"    output.vertexColor.rgb = materialEmissive.rgb + materialAmbient.rgb * ambient;\n"
"    output.vertexColor.a = 1.0f;\n"
"#if GTE_USE_MAT_VEC\n"
"    output.clipPosition = mul(pvwMatrix, float4(input.modelPosition, 1.0f));\n"
"#else\n"
"    output.clipPosition = mul(float4(input.modelPosition, 1.0f), pvwMatrix);\n"
"#endif\n"
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

std::string const* AmbientLightEffect::msVSSource[] =
{
    &msGLSLVSSource,
    &msHLSLSource
};

std::string const* AmbientLightEffect::msPSSource[] =
{
    &msGLSLPSSource,
    &msHLSLSource
};
