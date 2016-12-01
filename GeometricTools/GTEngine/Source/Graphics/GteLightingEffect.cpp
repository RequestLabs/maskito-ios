// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Graphics/GteLightingEffect.h>
#include <Mathematics/GteMatrix4x4.h>
using namespace gte;

LightingEffect::LightingEffect(ProgramFactory& factory, BufferUpdater const& updater,
    std::string const* vsSource[], std::string const* psSource[],
    std::shared_ptr<Material> const& material, std::shared_ptr<Lighting> const& lighting,
    std::shared_ptr<LightCameraGeometry> const& geometry)
    :
    mMaterial(material),
    mLighting(lighting),
    mGeometry(geometry)
{
    int api = factory.GetAPI();
    mProgram = factory.CreateFromSources(*vsSource[api], *psSource[api], "");
    if (mProgram)
    {
        mBufferUpdater = updater;
        mPVWMatrixConstant = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
        mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
    }
}

void LightingEffect::UpdateMaterialConstant()
{
    if (mMaterialConstant)
    {
        mBufferUpdater(mMaterialConstant);
    }
}

void LightingEffect::UpdateLightingConstant()
{
    if (mLightingConstant)
    {
        mBufferUpdater(mLightingConstant);
    }
}

void LightingEffect::UpdateGeometryConstant()
{
    if (mGeometryConstant)
    {
        mBufferUpdater(mGeometryConstant);
    }
}
