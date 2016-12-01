// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteProgramFactory.h>
#include <Graphics/GteTexture3.h>

#if defined(GTE_DEV_OPENGL)
#include <Graphics/GL4/GteGL4Engine.h>
#else
#include <Graphics/DX11/GteDX11Engine.h>
#endif

namespace gte
{

class GTE_IMPEXP Fluid3InitializeState
{
public:
    // Construction and destruction.  The initial velocity is zero and the
    // initial density is randomly generated with values in [0,1].
    ~Fluid3InitializeState();
    Fluid3InitializeState(ProgramFactory& factory, int xSize, int ySize,
        int zSize, int numXThreads, int numYThreads, int numZThreads);

    // Member access.  The texels are (velocity.xyz, density).
    inline std::shared_ptr<Texture3> const& GetStateTm1() const;
    inline std::shared_ptr<Texture3> const& GetStateT() const;

    // Compute the initial density and initial velocity for the fluid
    // simulation.
    void Execute(
#if defined(GTE_DEV_OPENGL)
        GL4Engine* engine
#else
        DX11Engine* engine
#endif
        );

private:
    int mNumXGroups, mNumYGroups, mNumZGroups;
    std::shared_ptr<ComputeProgram> mInitializeState;
    std::shared_ptr<Texture3> mDensity;
    std::shared_ptr<Texture3> mVelocity;
    std::shared_ptr<Texture3> mStateTm1;
    std::shared_ptr<Texture3> mStateT;

    // Shader source code as strings.
    static std::string const msGLSLSource;
    static std::string const msHLSLSource;
    static std::string const* msSource[ProgramFactory::PF_NUM_API];
};


inline std::shared_ptr<Texture3> const& Fluid3InitializeState::GetStateTm1()
    const
{
    return mStateTm1;
}

inline std::shared_ptr<Texture3> const& Fluid3InitializeState::GetStateT()
    const
{
    return mStateT;
}


}
