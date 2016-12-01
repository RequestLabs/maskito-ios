// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngineDEF.h>

// The ComputeModel class allows you to select the type of hardware to use
// in your computational algorithms.
//
// If your computational algorithm requires the GPU, set 'inEngine' to the
// object that will execute the compute shaders.  If your algorithm
// requires CPU multithreading, set the 'inNumThreads' to the desired
// number of threads, presumably 2 or larger.  You can query for the number
// of concurrent hardware threads using std::thread::hardware_concurrency().
// If you want single-threaded computations (on the main thread), set
// inNumThreads to 1.  An example of using this class is
//
//  ComputeModel cmodel(...);
//  if (cmodel.engine)
//  {
//      ComputeUsingGPU(...);
//  }
//  else if (cmodel.numThreads > 1)
//  {
//      ComputeUsingCPUMultipleThreads();
//  }
//  else
//  {
//      ComputeUsingCPUSingleThread();
//  }
// See GenerateMeshUV<Real>::SolveSystem(...) for a concrete example.
//
// Of course, your algorithm can interpret cmodel anyway it likes.  For
// example, you might ignore cmodel.engine if all you care about is
// multithreading on the CPU.

#if defined(WIN32)

#if defined(GTE_DEV_OPENGL)
#include <Graphics/GL4/GteGL4Engine.h>
#include <Graphics/GL4/GteGLSLProgramFactory.h>
#else
#include <Graphics/DX11/GteDX11Engine.h>
#include <Graphics/DX11/GteHLSLProgramFactory.h>
#endif

namespace gte
{

class GTE_IMPEXP ComputeModel
{
public:
    // Construction and destruction.  You may derive from this class to make
    // additional behavior available to your algorithms.  For example, you
    // might add a callback function to report progress of the algorithm.
    virtual ~ComputeModel();
    ComputeModel();
    ComputeModel(unsigned int inNumThreads);
    unsigned int numThreads;

    ComputeModel(
#if defined(GTE_DEV_OPENGL)
        GL4Engine* inEngine,
#else
        DX11Engine* inEngine,
#endif
        ProgramFactory* inFactory, unsigned int inNumThreads
    );

#if defined(GTE_DEV_OPENGL)
    GL4Engine* engine;
#else
    DX11Engine* engine;
#endif
    ProgramFactory* factory;
};

}

#else

// TODO: Add support for OpenGL GPGPU.

namespace gte
{

class GTE_IMPEXP ComputeModel
{
public:
    // Construction and destruction.  You may derive from this class to make
    // additional behavior available to your algorithms.  For example, you
    // might add a callback function to report progress of the algorithm.
    virtual ~ComputeModel();
    ComputeModel();
    ComputeModel(unsigned int inNumThreads);
    unsigned int numThreads;
};

}

#endif
