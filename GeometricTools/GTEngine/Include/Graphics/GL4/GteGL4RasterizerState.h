// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteRasterizerState.h>
#include <Graphics/GL4/GteGL4DrawingState.h>

namespace gte
{

class GTE_IMPEXP GL4RasterizerState : public GL4DrawingState
{
public:
    // Construction and destruction.
    virtual ~GL4RasterizerState();
    GL4RasterizerState(RasterizerState const* rasterizerState);
    static GL4GraphicsObject* Create(GraphicsObject const* object);

    // Member access.
    RasterizerState* GetRasterizerState();

    // Enable the rasterizer state.
    void Enable(HGLRC context);

private:
    GLenum mFillMode;
    GLenum mCullFace;
    GLenum mFrontFace;
    float mDepthScale;
    float mDepthBias;
    GLboolean mEnableScissor;

    // TODO: D3D11_RASTERIZER_DESC has the following.  We need to determine
    // how to handle these in OpenGL.
    //   DepthBiasClamp
    //   DepthClipEnable
    //   MultisampleEnable
    //   AntialiasedLineEnable

    // Conversions from GTEngine values to GL4 values.
    static GLenum const msFillMode[];
    static GLenum const msCullFace[];
};

}
