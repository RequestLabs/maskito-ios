// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
using namespace gte;

class MultipleRenderTargetsWindow : public Window3
{
public:
    MultipleRenderTargetsWindow(Parameters& parameters);

    virtual void OnIdle();

    virtual bool OnCharPress(unsigned char key, int x, int y);

private:
    bool SetEnvironment();
    bool CreateScene();
    void InitializeCamera();
    void CreateOverlays();

    std::shared_ptr<DrawTarget> mDrawTarget;
    std::shared_ptr<Visual> mSquare;
    std::shared_ptr<Texture2> mLinearDepth;
    std::shared_ptr<OverlayEffect> mOverlay[7];
    unsigned int mActiveOverlay;

    // Shader source code as strings.
    static std::string const msGLSLOverlayPSSource[5];
    static std::string const msHLSLOverlayPSSource[5];
    static std::string const* msOverlayPSSource[ProgramFactory::PF_NUM_API][5];
};
