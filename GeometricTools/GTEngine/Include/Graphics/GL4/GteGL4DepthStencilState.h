// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteDepthStencilState.h>
#include <Graphics/GL4/GteGL4DrawingState.h>

namespace gte
{

class GTE_IMPEXP GL4DepthStencilState : public GL4DrawingState
{
public:
    // Construction and destruction.
    virtual ~GL4DepthStencilState();
    GL4DepthStencilState(DepthStencilState const* depthStencilState);
    static GL4GraphicsObject* Create(GraphicsObject const* object);

    // Member access.
    DepthStencilState* GetDepthStencilState();

    // Enable the depth-stencil state.
    void Enable(HGLRC context);

private:
    struct Face
    {
        GLenum onFail;
        GLenum onZFail;
        GLenum onZPass;
        GLenum comparison;
    };

    GLboolean mDepthEnable;
    GLboolean mWriteMask;
    GLenum mComparison;
    GLboolean mStencilEnable;
    GLuint mStencilReadMask;
    GLuint mStencilWriteMask;
    Face mFrontFace;
    Face mBackFace;
    GLuint mReference;

    // Conversions from GTEngine values to GL4 values.
    static GLboolean const msWriteMask[];
    static GLenum const msComparison[];
    static GLenum const msOperation[];
};

}
