// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Physics/GteFluid2Parameters.h>
#include <Graphics/GteProgramFactory.h>
#include <Graphics/GteTexture2.h>

#if defined(GTE_DEV_OPENGL)
#include <Graphics/GL4/GteGL4Engine.h>
#else
#include <Graphics/DX11/GteDX11Engine.h>
#endif

namespace gte
{

class GTE_IMPEXP Fluid2ComputeDivergence
{
public:
    // Construction and destruction.  Compute the divergence of the velocity
    // vector field:  Divergence(V(x,y)) = dV/dx + dV/dy.  The derivatives are
    // estimated using centered finite differences,
    //   dV/dx = (V(x+dx,y) - V(x-dx,y))/(2*dx)
    //   dV/dy = (V(x,y+dy) - V(x,y-dy))/(2*dy)
    ~Fluid2ComputeDivergence();
    Fluid2ComputeDivergence(ProgramFactory& factory,
        int xSize, int ySize, int numXThreads, int numYThreads,
        std::shared_ptr<ConstantBuffer> const& parameters);

    // Member access.
    inline std::shared_ptr<Texture2> const& GetDivergence() const;

    // Compute the divergence of the velocity vector field for the input
    // state.
    void Execute(
#if defined(GTE_DEV_OPENGL)
        GL4Engine* engine,
#else
        DX11Engine* engine,
#endif
        std::shared_ptr<Texture2> const& state);

private:
    int mNumXGroups, mNumYGroups;
    std::shared_ptr<ComputeProgram> mComputeDivergence;
    std::shared_ptr<Texture2> mDivergence;

    // Shader source code as strings.
    static std::string const msGLSLSource;
    static std::string const msHLSLSource;
    static std::string const* msSource[ProgramFactory::PF_NUM_API];
};


inline std::shared_ptr<Texture2> const&
Fluid2ComputeDivergence::GetDivergence() const
{
    return mDivergence;
}


}
