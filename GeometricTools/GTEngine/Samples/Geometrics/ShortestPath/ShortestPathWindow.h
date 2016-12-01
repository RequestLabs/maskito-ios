// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
using namespace gte;

//#define USE_CPU_SHORTEST_PATH
#if defined(USE_CPU_SHORTEST_PATH)
#include "CpuShortestPath.h"
#else
#include "GpuShortestPath.h"
#endif

class ShortestPathWindow : public Window
{
public:
    // The application window is square and the height field for the
    // graph is square, both with this dimension.
    enum
    {
        ISIZE = 512,
        LOGISIZE = 9
    };

    ShortestPathWindow(Parameters& parameters);

    virtual void OnIdle();

private:
    bool SetEnvironment();
    void CreateWeightsShader();
    void GenerateWeights();
    std::shared_ptr<ConstantBuffer> CreateBicubicMatrix();
    void DrawPath(std::stack<std::pair<int, int>>& path);

    std::shared_ptr<OverlayEffect> mOverlay;
    std::shared_ptr<Texture2> mRandom;
    std::shared_ptr<Texture2> mWeights;
    std::shared_ptr<ComputeProgram> mWeightsProgram;
    unsigned int mNumGroups;

#if defined(USE_CPU_SHORTEST_PATH)
    std::shared_ptr<CpuShortestPath> mCpuShortestPath;
#else
    std::shared_ptr<GpuShortestPath> mGpuShortestPath;
#endif
};
