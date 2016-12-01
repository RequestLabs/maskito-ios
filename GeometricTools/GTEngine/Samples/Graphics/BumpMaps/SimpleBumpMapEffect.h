// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Mathematics/GteMatrix4x4.h>
#include <Mathematics/GteVector2.h>
#include <Graphics/GteTexture2.h>
#include <Graphics/GteVisual.h>
#include <Graphics/GteVisualEffect.h>
#include <Applications/GteEnvironment.h>

namespace gte
{

class SimpleBumpMapEffect : public VisualEffect
{
public:
    // Construction.
    SimpleBumpMapEffect(ProgramFactory& factory,
        Environment const& environment, bool& created);

    // Member access.
    inline void SetPVWMatrix(Matrix4x4<float> const& pvwMatrix);
    inline Matrix4x4<float> const& GetPVWMatrix() const;

    // Required to bind and update resources.
    inline std::shared_ptr<ConstantBuffer> const& GetPVWMatrixConstant() const;
    inline std::shared_ptr<Texture2> const& GetBaseTexture() const;
    inline std::shared_ptr<Texture2> const& GetNormalTexture() const;
    inline std::shared_ptr<SamplerState> const& GetCommonSampler() const;

    // The 'mesh' is one to which an instance of this effect is attached.
    // TODO: Move this into a compute shader to improve performance.
    static void ComputeLightVectors(std::shared_ptr<Visual> const& mesh,
        Vector4<float> const& worldLightDirection);

private:
    // Compute a tangent at the vertex P0.  The triangle is counterclockwise
    // ordered, <P0,P1,P2>.
    static bool ComputeTangent(
        Vector3<float> const& position0, Vector2<float> const& tcoord0,
        Vector3<float> const& position1, Vector2<float> const& tcoord1,
        Vector3<float> const& position2, Vector2<float> const& tcoord2,
        Vector3<float>& tangent);

    // Vertex shader parameters.
    std::shared_ptr<ConstantBuffer> mPVWMatrixConstant;

    // Pixel shader parameters.
    std::shared_ptr<Texture2> mBaseTexture;
    std::shared_ptr<Texture2> mNormalTexture;
    std::shared_ptr<SamplerState> mCommonSampler;

    // Convenience pointer.
    Matrix4x4<float>* mPVWMatrix;
};


inline void SimpleBumpMapEffect::SetPVWMatrix(Matrix4x4<float> const& pvwMatrix)
{
    *mPVWMatrix = pvwMatrix;
}

inline Matrix4x4<float> const& SimpleBumpMapEffect::GetPVWMatrix() const
{
    return *mPVWMatrix;
}

inline std::shared_ptr<ConstantBuffer> const& SimpleBumpMapEffect::GetPVWMatrixConstant() const
{
    return mPVWMatrixConstant;
}

inline std::shared_ptr<Texture2> const& SimpleBumpMapEffect::GetBaseTexture() const
{
    return mBaseTexture;
}

inline std::shared_ptr<Texture2> const& SimpleBumpMapEffect::GetNormalTexture() const
{
    return mNormalTexture;
}

inline std::shared_ptr<SamplerState> const& SimpleBumpMapEffect::GetCommonSampler() const
{
    return mCommonSampler;
}

}
