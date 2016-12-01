// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Mathematics/GteMatrix4x4.h>
#include <Graphics/GteVisualEffect.h>

namespace gte
{

class GTE_IMPEXP ConstantColorEffect : public VisualEffect
{
public:
    // Construction.
    ConstantColorEffect(ProgramFactory& factory, Vector4<float> const& color);

    // Member access.
    inline void SetPVWMatrix(Matrix4x4<float> const& pvwMatrix);
    inline Matrix4x4<float> const& GetPVWMatrix() const;

    // Required to bind and update resources.
    inline std::shared_ptr<ConstantBuffer> const& GetPVWMatrixConstant() const;
    inline std::shared_ptr<ConstantBuffer> const& GetColorConstant() const;

private:
    // Vertex shader parameters.
    std::shared_ptr<ConstantBuffer> mPVWMatrixConstant;
    std::shared_ptr<ConstantBuffer> mColorConstant;

    // Convenience pointers.
    Matrix4x4<float>* mPVWMatrix;
    Vector4<float>* mColor;

    // Shader source code as strings.
    static std::string const msGLSLVSSource;
    static std::string const msGLSLPSSource;
    static std::string const msHLSLSource;
    static std::string const* msVSSource[ProgramFactory::PF_NUM_API];
    static std::string const* msPSSource[ProgramFactory::PF_NUM_API];
};


inline void ConstantColorEffect::SetPVWMatrix(
    Matrix4x4<float> const& pvwMatrix)
{
    *mPVWMatrix = pvwMatrix;
}

inline Matrix4x4<float> const& ConstantColorEffect::GetPVWMatrix() const
{
    return *mPVWMatrix;
}

inline std::shared_ptr<ConstantBuffer> const&
ConstantColorEffect::GetPVWMatrixConstant() const
{
    return mPVWMatrixConstant;
}

inline std::shared_ptr<ConstantBuffer> const&
ConstantColorEffect::GetColorConstant() const
{
    return mColorConstant;
}


}
