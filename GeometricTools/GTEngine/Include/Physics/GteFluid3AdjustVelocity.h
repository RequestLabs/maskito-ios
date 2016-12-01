// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Physics/GteFluid3Parameters.h>
#include <Graphics/GteProgramFactory.h>
#include <Graphics/GteTexture3.h>

#if defined(GTE_DEV_OPENGL)
#include <Graphics/GL4/GteGL4Engine.h>
#else
#include <Graphics/DX11/GteDX11Engine.h>
#endif

namespace gte
{

class GTE_IMPEXP Fluid3AdjustVelocity
{
public:
    // Construction and destruction.  Adjust the velocities using the
    // solution to the Poisson equation.
    ~Fluid3AdjustVelocity();
    Fluid3AdjustVelocity(ProgramFactory& factory, int xSize, int ySize,
        int zSize, int numXThreads, int numYThreads, int numZThreads,
        std::shared_ptr<ConstantBuffer> const& parameters);

    // Update the state for the fluid simulation.
    void Execute(
#if defined(GTE_DEV_OPENGL)
        GL4Engine* engine,
#else
        DX11Engine* engine,
#endif
        std::shared_ptr<Texture3> const& inState,
        std::shared_ptr<Texture3> const& poisson,
        std::shared_ptr<Texture3> const& outState);

private:
    int mNumXGroups, mNumYGroups, mNumZGroups;
    std::shared_ptr<ComputeProgram> mAdjustVelocity;

    // Shader source code as strings.
    static std::string const msGLSLSource;
    static std::string const msHLSLSource;
    static std::string const* msSource[ProgramFactory::PF_NUM_API];
};

}
