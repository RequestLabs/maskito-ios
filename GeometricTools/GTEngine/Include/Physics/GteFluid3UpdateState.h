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

class GTE_IMPEXP Fluid3UpdateState
{
public:
    // Construction and destruction.
    ~Fluid3UpdateState();
    Fluid3UpdateState(ProgramFactory& factory, int xSize, int ySize,
        int zSize, int numXThreads, int numYThreads, int numZThreads,
        std::shared_ptr<ConstantBuffer> const& parameters);

    // Member access.  The texels are (velocity.xyz, density).
    inline std::shared_ptr<Texture3> const& GetUpdateState() const;

    // Update the state for the fluid simulation.
    void Execute(
#if defined(GTE_DEV_OPENGL)
        GL4Engine* engine,
#else
        DX11Engine* engine,
#endif
        std::shared_ptr<Texture3> const& source,
        std::shared_ptr<Texture3> const& stateTm1,
        std::shared_ptr<Texture3> const& stateT);

private:
    int mNumXGroups, mNumYGroups, mNumZGroups;
    std::shared_ptr<ComputeProgram> mComputeUpdateState;
    std::shared_ptr<SamplerState> mAdvectionSampler;
    std::shared_ptr<Texture3> mUpdateState;

    // Shader source code as strings.
    static std::string const msGLSLSource;
    static std::string const msHLSLSource;
    static std::string const* msSource[ProgramFactory::PF_NUM_API];
};


inline std::shared_ptr<Texture3> const& Fluid3UpdateState::GetUpdateState()
    const
{
    return mUpdateState;
}


}
