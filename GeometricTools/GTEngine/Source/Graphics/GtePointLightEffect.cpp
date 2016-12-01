// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Graphics/GtePointLightEffect.h>
using namespace gte;

PointLightEffect::PointLightEffect(
    ProgramFactory& factory, BufferUpdater const& updater, int select,
    std::shared_ptr<Material> const& material, std::shared_ptr<Lighting> const& lighting,
    std::shared_ptr<LightCameraGeometry> const& geometry)
    :
    LightingEffect(factory, updater, msVSSource[select & 1], msPSSource[select & 1],
        material, lighting, geometry)
{
    mMaterialConstant = std::make_shared<ConstantBuffer>(sizeof(InternalMaterial), true);
    UpdateMaterialConstant();

    mLightingConstant = std::make_shared<ConstantBuffer>(sizeof(InternalLighting), true);
    UpdateLightingConstant();

    mGeometryConstant = std::make_shared<ConstantBuffer>(sizeof(InternalGeometry), true);
    UpdateGeometryConstant();

    if ((select & 1) == 0)
    {
        mProgram->GetVShader()->Set("Material", mMaterialConstant);
        mProgram->GetVShader()->Set("Lighting", mLightingConstant);
        mProgram->GetVShader()->Set("LightCameraGeometry", mGeometryConstant);
    }
    else
    {
        mProgram->GetPShader()->Set("Material", mMaterialConstant);
        mProgram->GetPShader()->Set("Lighting", mLightingConstant);
        mProgram->GetPShader()->Set("LightCameraGeometry", mGeometryConstant);
    }
}

void PointLightEffect::UpdateMaterialConstant()
{
    InternalMaterial* internalMaterial = mMaterialConstant->Get<InternalMaterial>();
    internalMaterial->emissive = mMaterial->emissive;
    internalMaterial->ambient = mMaterial->ambient;
    internalMaterial->diffuse = mMaterial->diffuse;
    internalMaterial->specular = mMaterial->specular;
    LightingEffect::UpdateMaterialConstant();
}

void PointLightEffect::UpdateLightingConstant()
{
    InternalLighting* internalLighting = mLightingConstant->Get<InternalLighting>();
    internalLighting->ambient = mLighting->ambient;
    internalLighting->diffuse = mLighting->diffuse;
    internalLighting->specular = mLighting->specular;
    internalLighting->attenuation = mLighting->attenuation;
    LightingEffect::UpdateLightingConstant();
}

void PointLightEffect::UpdateGeometryConstant()
{
    InternalGeometry* internalGeometry = mGeometryConstant->Get<InternalGeometry>();
    internalGeometry->lightModelPosition = mGeometry->lightModelPosition;
    internalGeometry->cameraModelPosition = mGeometry->cameraModelPosition;
    LightingEffect::UpdateGeometryConstant();
}

// TODO:  Write these shaders.
std::string const PointLightEffect::msGLSLVSSource[2] = { "", "" };
std::string const PointLightEffect::msGLSLPSSource[2] = { "", "" };

std::string const PointLightEffect::msHLSLSource[2] =
{
    "cbuffer PVWMatrix\n"
    "{\n"
    "    float4x4 pvwMatrix;\n"
    "};\n"
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
    "    float4 lightModelPosition;\n"
    "    float4 cameraModelPosition;\n"
    "};\n"
    "\n"
    "struct VS_INPUT\n"
    "{\n"
    "    float3 modelPosition : POSITION;\n"
    "    float3 modelNormal : NORMAL;\n"
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
    "    float3 modelLightDiff = input.modelPosition - lightModelPosition.xyz;\n"
    "    float3 vertexDirection = normalize(modelLightDiff);\n"
    "    float NDotL = -dot(input.modelNormal, vertexDirection);\n"
    "    float3 viewVector = normalize(cameraModelPosition.xyz - input.modelPosition);\n"
    "    float3 halfVector = normalize(viewVector - vertexDirection);\n"
    "    float NDotH = dot(input.modelNormal, halfVector);\n"
    "    float4 lighting = lit(NDotL, NDotH, materialSpecular.a);\n"
    "\n"
    "    float distance = length(modelLightDiff);\n"
    "    float attenuation = lightingAttenuation.w / (lightingAttenuation.x + distance *\n"
    "        (lightingAttenuation.y + distance * lightingAttenuation.z));\n"
    "\n"
    "    float3 color = materialAmbient.rgb * lightingAmbient.rgb +\n"
    "        lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +\n"
    "        lighting.z * materialSpecular.rgb * lightingSpecular.rgb;\n"
    "\n"
    "    output.vertexColor.rgb = materialEmissive.rgb + attenuation * color;\n"
    "    output.vertexColor.a = materialDiffuse.a;\n"
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
    "}\n",

    "cbuffer PVWMatrix\n"
    "{\n"
    "    float4x4 pvwMatrix;\n"
    "};\n"
    "\n"
    "struct VS_INPUT\n"
    "{\n"
    "    float3 modelPosition : POSITION;\n"
    "    float3 modelNormal : NORMAL;\n"
    "};\n"
    "\n"
    "struct VS_OUTPUT\n"
    "{\n"
    "    float3 vertexPosition : TEXCOORD0;\n"
    "    float3 vertexNormal : TEXCOORD1;\n"
    "    float4 clipPosition : SV_POSITION;\n"
    "};\n"
    "\n"
    "VS_OUTPUT VSMain(VS_INPUT input)\n"
    "{\n"
    "    VS_OUTPUT output;\n"
    "\n"
    "    output.vertexPosition = input.modelPosition;\n"
    "    output.vertexNormal = input.modelNormal;\n"
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
    "    float4 lightModelPosition;\n"
    "    float4 cameraModelPosition;\n"
    "};\n"
    "\n"
    "struct PS_INPUT\n"
    "{\n"
    "    float3 vertexPosition : TEXCOORD0;\n"
    "    float3 vertexNormal : TEXCOORD1;\n"
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
    "    float3 modelLightDiff = input.vertexPosition - lightModelPosition.xyz;\n"
    "    float3 vertexDirection = normalize(modelLightDiff);\n"
    "    float NDotL = -dot(input.vertexNormal, vertexDirection);\n"
    "    float3 viewVector = normalize(cameraModelPosition.xyz - input.vertexPosition.xyz);\n"
    "    float3 halfVector = normalize(viewVector - vertexDirection);\n"
    "    float NDotH = dot(input.vertexNormal, halfVector);\n"
    "    float4 lighting = lit(NDotL, NDotH, materialSpecular.a);\n"
    "\n"
    "    float distance = length(modelLightDiff);\n"
    "    float attenuation = lightingAttenuation.w / (lightingAttenuation.x + distance *\n"
    "        (lightingAttenuation.y + distance * lightingAttenuation.z));\n"
    "\n"
    "    float3 color = materialAmbient.rgb * lightingAmbient.rgb +\n"
    "        lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +\n"
    "        lighting.z * materialSpecular.rgb * lightingSpecular.rgb;\n"
    "\n"
    "    output.pixelColor0.rgb = materialEmissive.rgb + attenuation * color;\n"
    "    output.pixelColor0.a = materialDiffuse.a;\n"
    "    return output;\n"
    "}\n"
};

std::string const* PointLightEffect::msVSSource[2][ProgramFactory::PF_NUM_API] =
{
    {
        &msGLSLVSSource[0],
        &msHLSLSource[0]
    },
    {
        &msGLSLVSSource[1],
        &msHLSLSource[1]
    }
};

std::string const* PointLightEffect::msPSSource[2][ProgramFactory::PF_NUM_API] =
{
    {
        &msGLSLPSSource[0],
        &msHLSLSource[0]
    },
    {
        &msGLSLPSSource[1],
        &msHLSLSource[1]
    }
};
