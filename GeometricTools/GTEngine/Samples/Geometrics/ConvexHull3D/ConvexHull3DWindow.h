// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
using namespace gte;

class ConvexHull3DWindow : public Window3
{
public:
    ConvexHull3DWindow(Parameters& parameters);

    virtual void OnIdle();

    virtual bool OnCharPress(unsigned char key, int x, int y);

private:
    bool SetEnvironment();
    bool LoadData();
    void InitializeCamera();

    // The input data files are in the Data subfolder.  The files are of the
    // format "dataXX.txt", where XX is in {01,02,...,46}.
    int mFileQuantity;  // = 46
    int mCurrentFile;  // = 1 initially

    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Visual> mMesh;
    std::shared_ptr<VertexColorEffect> mEffect;
    std::string mMessage;
};
