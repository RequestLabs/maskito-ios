// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteProgramFactory.h>
#include <Graphics/GteTexture2.h>

#if defined(GTE_DEV_OPENGL)
#include <Graphics/GL4/GteGL4Engine.h>
#else
#include <Graphics/DX11/GteDX11Engine.h>
#endif

namespace gte
{

class GTE_IMPEXP Fluid2InitializeState
{
public:
    // Construction and destruction.  The initial velocity is zero and the
    // initial density is randomly generated with values in [0,1].
    ~Fluid2InitializeState();
    Fluid2InitializeState(ProgramFactory& factory,
        int xSize, int ySize, int numXThreads, int numYThreads);

    // Member access.  The texels are (velocity.x, velocity.y, 0, density).
    // The third component is unused in the simulation (a 3D simulation will
    // store velocity.z in this component).
    inline std::shared_ptr<Texture2> const& GetStateTm1() const;
    inline std::shared_ptr<Texture2> const& GetStateT() const;

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
    int mNumXGroups, mNumYGroups;
    std::shared_ptr<ComputeProgram> mInitializeState;
    std::shared_ptr<Texture2> mDensity;
    std::shared_ptr<Texture2> mVelocity;
    std::shared_ptr<Texture2> mStateTm1;
    std::shared_ptr<Texture2> mStateT;

    // Shader source code as strings.
    static std::string const msGLSLSource;
    static std::string const msHLSLSource;
    static std::string const* msSource[ProgramFactory::PF_NUM_API];
};


inline std::shared_ptr<Texture2> const& Fluid2InitializeState::GetStateTm1()
    const
{
    return mStateTm1;
}

inline std::shared_ptr<Texture2> const& Fluid2InitializeState::GetStateT()
    const
{
    return mStateT;
}


}
