// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#if defined(WIN32)
// Fluid2
#include <Physics/GteFluid2.h>
#include <Physics/GteFluid2AdjustVelocity.h>
#include <Physics/GteFluid2ComputeDivergence.h>
#include <Physics/GteFluid2EnforceStateBoundary.h>
#include <Physics/GteFluid2InitializeSource.h>
#include <Physics/GteFluid2InitializeState.h>
#include <Physics/GteFluid2Parameters.h>
#include <Physics/GteFluid2SolvePoisson.h>
#include <Physics/GteFluid2UpdateState.h>

// Fluid3
#include <Physics/GteFluid3.h>
#include <Physics/GteFluid3AdjustVelocity.h>
#include <Physics/GteFluid3ComputeDivergence.h>
#include <Physics/GteFluid3EnforceStateBoundary.h>
#include <Physics/GteFluid3InitializeSource.h>
#include <Physics/GteFluid3InitializeState.h>
#include <Physics/GteFluid3Parameters.h>
#include <Physics/GteFluid3SolvePoisson.h>
#include <Physics/GteFluid3UpdateState.h>
#endif

// ParticleSystems
#include <Physics/GteMassSpringArbitrary.h>
#include <Physics/GteMassSpringCurve.h>
#include <Physics/GteMassSpringSurface.h>
#include <Physics/GteMassSpringVolume.h>
#include <Physics/GteParticleSystem.h>

// RigidBody
#include <Physics/GtePolyhedralMassProperties.h>
#include <Physics/GteRigidBody.h>
