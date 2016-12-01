// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
using namespace gte;

class WireMeshWindow : public Window3
{
public:
    WireMeshWindow(Parameters& parameters);

    virtual void OnIdle();

private:
    bool SetEnvironment();
    bool CreateScene();
    void InitializeCamera();

    std::shared_ptr<Visual> mMesh;
};
