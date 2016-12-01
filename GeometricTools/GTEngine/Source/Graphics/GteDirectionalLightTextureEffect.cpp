// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Graphics/GteDirectionalLightTextureEffect.h>
using namespace gte;

DirectionalLightTextureEffect::DirectionalLightTextureEffect(ProgramFactory& factory, BufferUpdater const& updater,
    std::shared_ptr<Material> const& material, std::shared_ptr<Lighting> const& lighting,
    std::shared_ptr<LightCameraGeometry> const& geometry, std::shared_ptr<Texture2> const& texture,
    SamplerState::Filter filter, SamplerState::Mode mode0, SamplerState::Mode mode1)
    :
    LightingEffect(factory, updater, msVSSource, msPSSource, material, lighting, geometry),
    mTexture(texture)
{
    mSampler = std::make_shared<SamplerState>();
    mSampler->filter = filter;
    mSampler->mode[0] = mode0;
    mSampler->mode[1] = mode1;

    mMaterialConstant = std::make_shared<ConstantBuffer>(sizeof(InternalMaterial), true);
    UpdateMaterialConstant();

    mLightingConstant = std::make_shared<ConstantBuffer>(sizeof(InternalLighting), true);
    UpdateLightingConstant();

    mGeometryConstant = std::make_shared<ConstantBuffer>(sizeof(InternalGeometry), true);
    UpdateGeometryConstant();

    mProgram->GetPShader()->Set("Material", mMaterialConstant);
    mProgram->GetPShader()->Set("Lighting", mLightingConstant);
    mProgram->GetPShader()->Set("LightCameraGeometry", mGeometryConstant);
    mProgram->GetPShader()->Set("baseTexture", mTexture);
    mProgram->GetPShader()->Set("baseSampler", mSampler);
}

void DirectionalLightTextureEffect::UpdateMaterialConstant()
{
    InternalMaterial* internalMaterial = mMaterialConstant->Get<InternalMaterial>();
    internalMaterial->emissive = mMaterial->emissive;
    internalMaterial->ambient = mMaterial->ambient;
    internalMaterial->diffuse = mMaterial->diffuse;
    internalMaterial->specular = mMaterial->specular;
    LightingEffect::UpdateMaterialConstant();
}

void DirectionalLightTextureEffect::UpdateLightingConstant()
{
    InternalLighting* internalLighting = mLightingConstant->Get<InternalLighting>();
    internalLighting->ambient = mLighting->ambient;
    internalLighting->diffuse = mLighting->diffuse;
    internalLighting->specular = mLighting->specular;
    internalLighting->attenuation = mLighting->attenuation;
    LightingEffect::UpdateLightingConstant();
}

void DirectionalLightTextureEffect::UpdateGeometryConstant()
{
    InternalGeometry* internalGeometry = mGeometryConstant->Get<InternalGeometry>();
    internalGeometry->lightModelDirection = mGeometry->lightModelDirection;
    internalGeometry->cameraModelPosition = mGeometry->cameraModelPosition;
    LightingEffect::UpdateGeometryConstant();
}

// TODO:  Write these shaders.
std::string const DirectionalLightTextureEffect::msGLSLVSSource = "";
std::string const DirectionalLightTextureEffect::msGLSLPSSource = "";

std::string const DirectionalLightTextureEffect::msHLSLSource =
"cbuffer PVWMatrix\n"
"{\n"
"    float4x4 pvwMatrix;\n"
"};\n"
"\n"
"struct VS_INPUT\n"
"{\n"
"    float3 modelPosition : POSITION;\n"
"    float3 modelNormal : NORMAL;\n"
"    float2 modelTCoord : TEXCOORD0;\n"
"};\n"
"\n"
"struct VS_OUTPUT\n"
"{\n"
"    float3 vertexPosition : TEXCOORD0;\n"
"    float3 vertexNormal : TEXCOORD1;\n"
"    float2 vertexTCoord : TEXCOORD2;\n"
"    float4 clipPosition : SV_POSITION;\n"
"};\n"
"\n"
"VS_OUTPUT VSMain(VS_INPUT input)\n"
"{\n"
"    VS_OUTPUT output;\n"
"\n"
"    output.vertexPosition = input.modelPosition;\n"
"    output.vertexNormal = input.modelNormal;\n"
"    output.vertexTCoord = input.modelTCoord;\n"
"#if GTE_USE_MAT_VEC\n"
"    output.clipPosition = mul(pvwMatrix, float4(input.modelPosition, 1.0f));\n"
"#else\n"
"    output.clipPosition = mul(float4(input.modelPosition, 1.0f), pvwMatrix);\n"
"#endif\n"
"    return output;\n"
"}\n"
"\n"
"cbuffer Material\n"
"{\n"
"    float4 materialEmissive;\n"
"    float4 materialAmbient;\n"
"    float4 materialDiffuse;\n"
"    float4 materialSpecular;\n"
"};\n"
"\n"
"cbuffer Lighting\n"
"{\n"
"    float4 lightingAmbient;\n"
"    float4 lightingDiffuse;\n"
"    float4 lightingSpecular;\n"
"    float4 lightingAttenuation;\n"
"};\n"
"\n"
"cbuffer LightCameraGeometry\n"
"{\n"
"    float4 lightModelDirection;\n"
"    float4 cameraModelPosition;\n"
"};\n"
"\n"
"Texture2D<float4> baseTexture;\n"
"SamplerState baseSampler;\n"
"\n"
"struct PS_INPUT\n"
"{\n"
"    float3 vertexPosition : TEXCOORD0;\n"
"    float3 vertexNormal : TEXCOORD1;\n"
"    float2 vertexTCoord : TEXCOORD2;\n"
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
"\n"
"    float3 normal = normalize(input.vertexNormal);\n"
"    float NDotL = -dot(normal, lightModelDirection.xyz);\n"
"    float3 viewVector = normalize(cameraModelPosition.xyz - input.vertexPosition);\n"
"    float3 halfVector = normalize(viewVector - lightModelDirection.xyz);\n"
"    float NDotH = dot(normal, halfVector);\n"
"    float4 lighting = lit(NDotL, NDotH, materialSpecular.a);\n"
"    float3 lightingColor = materialAmbient.rgb * lightingAmbient.rgb +\n"
"        lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +\n"
"        lighting.z * materialSpecular.rgb * lightingSpecular.rgb;\n"
"\n"
"    float4 textureColor = baseTexture.Sample(baseSampler, input.vertexTCoord);\n"
"\n"
"    float3 color = lightingColor * textureColor.rgb;\n"
"    output.pixelColor0.rgb = materialEmissive.rgb + lightingAttenuation.w * color;\n"
"    output.pixelColor0.a = materialDiffuse.a * textureColor.a;\n"
"    return output;\n"
"}\n";

std::string const* DirectionalLightTextureEffect::msVSSource[] =
{
    &msGLSLVSSource,
    &msHLSLSource
};

std::string const* DirectionalLightTextureEffect::msPSSource[] =
{
    &msGLSLPSSource,
    &msHLSLSource
};
