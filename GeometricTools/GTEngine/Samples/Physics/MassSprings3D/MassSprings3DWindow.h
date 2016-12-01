// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
using namespace gte;

//#define DO_CPU_MASS_SPRING
#if defined(DO_CPU_MASS_SPRING)
#include "CpuMassSpringVolume.h"
typedef CpuMassSpringVolume MassSpringSystem;
#else
#include "GpuMassSpringVolume.h"
typedef GpuMassSpringVolume MassSpringSystem;
#endif

class MassSprings3DWindow : public Window3
{
public:
    MassSprings3DWindow(Parameters& parameters);

    virtual void OnIdle();
    virtual bool OnCharPress(unsigned char key, int x, int y);

private:
    bool SetEnvironment();
    bool CreateMassSpringSystem();
    void InitializeCamera();
    void CreateBoxFaces();
    void UpdateTransforms();
    void UpdateMassSpringSystem();

    inline int GetIndex(int x, int y, int z) const
    {
        return x + mDimension[0] * (y + mDimension[1] * z);
    }

    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<VertexBuffer> mVBuffer;
    std::shared_ptr<Visual> mBoxFace[6];
    std::shared_ptr<VisualEffect> mEffect[6];
    std::unique_ptr<MassSpringSystem> mMassSprings;
    float mSimulationTime, mSimulationDelta;
    int mDimension[3];
};
