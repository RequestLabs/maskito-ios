// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
using namespace gte;

class ConvolutionWindow : public Window
{
public:
    ConvolutionWindow(Parameters& parameters);

    virtual void OnIdle();
    virtual bool OnCharPress(unsigned char key, int x, int y);

private:
    enum { MAX_RADIUS = 8 };

    bool SetEnvironment();
    void CreateShaders();
    void ExecuteShaders();
    std::shared_ptr<ConstantBuffer> GetKernel1(int radius);
    std::shared_ptr<ConstantBuffer> GetKernel2(int radius);

    std::shared_ptr<OverlayEffect> mOverlay[2];
    std::shared_ptr<Texture2> mImage[3];
    unsigned int mNumXGroups, mNumYGroups;
    int mRadius;

    // 0 = convolve
    // 1 = convolve groupshared
    // 2 = convolve separable
    // 3 = convolve separable groupshared (one slice at a time)
    // 4 = convolve separable groupshared (slice processed as subslices)
    unsigned int mSelection;
    static std::string msName[5];

    // selection 0
    std::shared_ptr<ComputeProgram> mConvolve;

    // selection 1
    std::shared_ptr<ComputeProgram> mConvolveGS;

    // selection 2
    std::shared_ptr<ComputeProgram> mConvolveSeparableH;
    std::shared_ptr<ComputeProgram> mConvolveSeparableV;

    // selection 3
    std::shared_ptr<ComputeProgram> mConvolveSeparableHGS;
    std::shared_ptr<ComputeProgram> mConvolveSeparableVGS;

    // selection 4
    std::shared_ptr<ComputeProgram> mConvolveSeparableHGS2;
    std::shared_ptr<ComputeProgram> mConvolveSeparableVGS2;
};
