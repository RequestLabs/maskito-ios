// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Mathematics/GteMatrix4x4.h>
#include <Mathematics/GteVector2.h>
#include <Graphics/GteTexture1.h>
#include <Graphics/GteTexture2.h>
#include <Graphics/GteVisualEffect.h>
#include <Applications/GteEnvironment.h>

namespace gte
{

class BlendedTerrainEffect : public VisualEffect
{
public:
    // Construction.
    BlendedTerrainEffect(ProgramFactory& factory,
        Environment const& environment, bool& created);

    // Member access.
    inline void SetPVWMatrix(Matrix4x4<float> const& pvwMatrix);
    inline Matrix4x4<float> const& GetPVWMatrix() const;
    inline void SetFlowDirection(Vector2<float> const& flowDirection);
    inline Vector2<float> const& GetFlowDirection() const;
    inline void SetPowerFactor(float powerFactor);
    inline float GetPowerFactor() const;

    // Required to bind and update resources.
    inline std::shared_ptr<ConstantBuffer> const& GetPVWMatrixConstant()
        const;
    inline std::shared_ptr<ConstantBuffer> const& GetFlowDirectionConstant()
        const;
    inline std::shared_ptr<ConstantBuffer> const& GetPowerFactorConstant()
        const;
    inline std::shared_ptr<Texture1> const& GetBlendTexture() const;
    inline std::shared_ptr<Texture2> const& GetGrassTexture() const;
    inline std::shared_ptr<Texture2> const& GetStoneTexture() const;
    inline std::shared_ptr<Texture2> const& GetCloudTexture() const;
    inline std::shared_ptr<SamplerState> const& GetCommonSampler() const;
    inline std::shared_ptr<SamplerState> const& GetBlendSampler() const;

private:
    // Vertex shader parameters.
    std::shared_ptr<ConstantBuffer> mPVWMatrixConstant;
    std::shared_ptr<ConstantBuffer> mFlowDirectionConstant;

    // Pixel shader parameters.
    std::shared_ptr<ConstantBuffer> mPowerFactorConstant;
    std::shared_ptr<Texture1> mBlendTexture;
    std::shared_ptr<Texture2> mGrassTexture;
    std::shared_ptr<Texture2> mStoneTexture;
    std::shared_ptr<Texture2> mCloudTexture;
    std::shared_ptr<SamplerState> mCommonSampler;
    std::shared_ptr<SamplerState> mBlendSampler;

    // Convenience pointers.
    Matrix4x4<float>* mPVWMatrix;
    Vector2<float>* mFlowDirection;
    float* mPowerFactor;
};


inline void BlendedTerrainEffect::SetPVWMatrix(Matrix4x4<float> const& pvwMatrix)
{
    *mPVWMatrix = pvwMatrix;
}

inline Matrix4x4<float> const& BlendedTerrainEffect::GetPVWMatrix() const
{
    return *mPVWMatrix;
}

inline void BlendedTerrainEffect::SetFlowDirection(Vector2<float> const& flowDirection)
{
    *mFlowDirection = flowDirection;
}

inline Vector2<float> const& BlendedTerrainEffect::GetFlowDirection() const
{
    return *mFlowDirection;
}

inline void BlendedTerrainEffect::SetPowerFactor(float powerFactor)
{
    *mPowerFactor = powerFactor;
}

inline float BlendedTerrainEffect::GetPowerFactor() const
{
    return *mPowerFactor;
}

inline std::shared_ptr<ConstantBuffer> const& BlendedTerrainEffect::GetPVWMatrixConstant() const
{
    return mPVWMatrixConstant;
}

inline std::shared_ptr<ConstantBuffer> const& BlendedTerrainEffect::GetFlowDirectionConstant() const
{
    return mFlowDirectionConstant;
}

inline std::shared_ptr<ConstantBuffer> const& BlendedTerrainEffect::GetPowerFactorConstant() const
{
    return mPowerFactorConstant;
}

inline std::shared_ptr<Texture1> const& BlendedTerrainEffect::GetBlendTexture() const
{
    return mBlendTexture;
}

inline std::shared_ptr<Texture2> const& BlendedTerrainEffect::GetGrassTexture() const
{
    return mGrassTexture;
}

inline std::shared_ptr<Texture2> const& BlendedTerrainEffect::GetStoneTexture() const
{
    return mStoneTexture;
}

inline std::shared_ptr<Texture2> const& BlendedTerrainEffect::GetCloudTexture() const
{
    return mCloudTexture;
}

inline std::shared_ptr<SamplerState> const& BlendedTerrainEffect::GetCommonSampler() const
{
    return mCommonSampler;
}

inline std::shared_ptr<SamplerState> const& BlendedTerrainEffect::GetBlendSampler() const
{
    return mBlendSampler;
}

}
