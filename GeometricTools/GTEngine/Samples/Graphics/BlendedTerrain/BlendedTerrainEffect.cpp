// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <Applications/GteWICFileIO.h>
#include "BlendedTerrainEffect.h"
using namespace gte;

BlendedTerrainEffect::BlendedTerrainEffect(ProgramFactory& factory,
    Environment const& environment, bool& created)
    :
    mPVWMatrix(nullptr),
    mFlowDirection(nullptr),
    mPowerFactor(nullptr)
{
    created = false;

    // Load and compile the shaders.
    std::string path = environment.GetPath("BlendedTerrain.hlsl");
#if !defined(GTE_DEV_OPENGL)
    // The flags are chosen to allow you to debug the shaders through MSVS.
    // The menu path is "Debug | Graphics | Start Diagnostics" (ALT+F5).
    factory.PushFlags();
    factory.flags = 
        D3DCOMPILE_ENABLE_STRICTNESS |
        D3DCOMPILE_IEEE_STRICTNESS |
        D3DCOMPILE_DEBUG |
        D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    mProgram = factory.CreateFromFiles(path, path, "");
#if !defined(GTE_DEV_OPENGL)
    factory.PopFlags();
#endif
    if (!mProgram)
    {
        // The program factory will generate Log* messages.
        return;
    }

    // Load the textures.
    path = environment.GetPath("BTGrass.png");
    mGrassTexture.reset(WICFileIO::Load(path, true));
    mGrassTexture->AutogenerateMipmaps();

    path = environment.GetPath("BTStone.png");
    mStoneTexture.reset(WICFileIO::Load(path, true));
    mStoneTexture->AutogenerateMipmaps();

    path = environment.GetPath("BTCloud.png");
    mCloudTexture.reset(WICFileIO::Load(path, true));
    mCloudTexture->AutogenerateMipmaps();

    // Create the shader constants.
    mPVWMatrixConstant = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    mPVWMatrix = mPVWMatrixConstant->Get<Matrix4x4<float>>();
    *mPVWMatrix = Matrix4x4<float>::Identity();

    mFlowDirectionConstant = std::make_shared<ConstantBuffer>(sizeof(Vector2<float>), true);
    mFlowDirection = mFlowDirectionConstant->Get<Vector2<float>>();
    *mFlowDirection = { 0.0f, 0.0f };

    mPowerFactorConstant = std::make_shared<ConstantBuffer>(sizeof(float), true);
    mPowerFactor = mPowerFactorConstant->Get<float>();
    *mPowerFactor = 1.0f;

    // Create a 1-dimensional texture whose intensities are proportional to
    // height.
    unsigned int const numTexels = 256;
    mBlendTexture = std::make_shared<Texture1>(DF_R8_UNORM, numTexels);
    unsigned char* texels = mBlendTexture->Get<unsigned char>();
    for (unsigned int i = 0; i < numTexels; ++i, ++texels)
    {
        *texels = static_cast<unsigned char>(i);
    }

    // Create the texture samplers.  The common sampler uses trilinear
    // interpolation (mipmapping).  The blend sample uses bilinear
    // interpolation (no mipmapping).
    mCommonSampler = std::make_shared<SamplerState>();
    mCommonSampler->filter = SamplerState::MIN_L_MAG_L_MIP_L;
    mCommonSampler->mode[0] = SamplerState::WRAP;
    mCommonSampler->mode[1] = SamplerState::WRAP;
    mBlendSampler = std::make_shared<SamplerState>();
    mBlendSampler->filter = SamplerState::MIN_L_MAG_L_MIP_P;
    mBlendSampler->mode[0] = SamplerState::WRAP;

    // Set the resources for the shaders.
    std::shared_ptr<VertexShader> vshader = mProgram->GetVShader();
    std::shared_ptr<PixelShader> pshader = mProgram->GetPShader();
    vshader->Set("PVWMatrix", mPVWMatrixConstant);
    vshader->Set("FlowDirection", mFlowDirectionConstant);
    pshader->Set("PowerFactor", mPowerFactorConstant);
    pshader->Set("grassTexture", mGrassTexture);
    pshader->Set("stoneTexture", mStoneTexture);
    pshader->Set("blendTexture", mBlendTexture);
    pshader->Set("cloudTexture", mCloudTexture);
    pshader->Set("commonSampler", mCommonSampler);
    pshader->Set("blendSampler", mBlendSampler);

    created = true;
}

