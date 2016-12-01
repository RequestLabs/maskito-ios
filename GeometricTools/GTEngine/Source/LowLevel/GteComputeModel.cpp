// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <LowLevel/GteComputeModel.h>
using namespace gte;

#if defined(WIN32)

ComputeModel::~ComputeModel()
{
}

ComputeModel::ComputeModel()
    :
    numThreads(1),
    engine(nullptr)
{
}

ComputeModel::ComputeModel(unsigned int inNumThreads)
    :
    numThreads(inNumThreads > 0 ? inNumThreads : 1),
    engine(nullptr),
    factory(nullptr)
{
}

ComputeModel::ComputeModel(
#if defined(GTE_DEV_OPENGL)
    GL4Engine* inEngine,
#else
    DX11Engine* inEngine,
#endif
    ProgramFactory* inFactory, unsigned int inNumThreads)
    :
    numThreads(inNumThreads > 0 ? inNumThreads : 1),
    engine(inEngine),
    factory(inFactory)
{
}

#else

ComputeModel::~ComputeModel()
{
}

ComputeModel::ComputeModel()
    :
    numThreads(1)
{
}

ComputeModel::ComputeModel(unsigned int inNumThreads)
    :
    numThreads(inNumThreads > 0 ? inNumThreads : 1)
{
}

#endif
