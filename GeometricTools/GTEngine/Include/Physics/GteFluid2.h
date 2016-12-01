// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Physics/GteFluid2AdjustVelocity.h>
#include <Physics/GteFluid2ComputeDivergence.h>
#include <Physics/GteFluid2EnforceStateBoundary.h>
#include <Physics/GteFluid2InitializeSource.h>
#include <Physics/GteFluid2InitializeState.h>
#include <Physics/GteFluid2SolvePoisson.h>
#include <Physics/GteFluid2UpdateState.h>

#if defined(GTE_DEV_OPENGL)
#include <Graphics/GL4/GteGL4Engine.h>
#include <Graphics/GL4/GteGLSLProgramFactory.h>
#else
#include <Graphics/DX11/GteDX11Engine.h>
#include <Graphics/DX11/GteHLSLProgramFactory.h>
#endif

namespace gte
{

class GTE_IMPEXP Fluid2
{
public:
    // Construction and destruction.  The (x,y) grid covers [0,1]^2.
    ~Fluid2();
    Fluid2(
#if defined(GTE_DEV_OPENGL)
        GL4Engine* engine,
        GLSLProgramFactory& factory,
#else
        DX11Engine* engine,
        HLSLProgramFactory& factory,
#endif
        int xSize, int ySize, float dt, float densityViscosity,
        float velocityViscosity);

    void Initialize();
    void DoSimulationStep();
    inline std::shared_ptr<Texture2> const& GetState () const;

private:
    // Constructor inputs.
#if defined(GTE_DEV_OPENGL)
    GL4Engine* mEngine;
#else
    DX11Engine* mEngine;
#endif
    int mXSize, mYSize;
    float mDt;

    // Current simulation time.
    float mTime;

    std::shared_ptr<ConstantBuffer> mParameters;
    std::shared_ptr<Fluid2InitializeSource> mInitializeSource;
    std::shared_ptr<Fluid2InitializeState> mInitializeState;
    std::shared_ptr<Fluid2EnforceStateBoundary> mEnforceStateBoundary;
    std::shared_ptr<Fluid2UpdateState> mUpdateState;
    std::shared_ptr<Fluid2ComputeDivergence> mComputeDivergence;
    std::shared_ptr<Fluid2SolvePoisson> mSolvePoisson;
    std::shared_ptr<Fluid2AdjustVelocity> mAdjustVelocity;

    std::shared_ptr<Texture2> mSourceTexture;
    std::shared_ptr<Texture2> mStateTm1Texture;
    std::shared_ptr<Texture2> mStateTTexture;
    std::shared_ptr<Texture2> mStateTp1Texture;
    std::shared_ptr<Texture2> mDivergenceTexture;
    std::shared_ptr<Texture2> mPoissonTexture;
};


inline std::shared_ptr<Texture2> const& Fluid2::GetState() const
{
    return mStateTTexture;
}


}
