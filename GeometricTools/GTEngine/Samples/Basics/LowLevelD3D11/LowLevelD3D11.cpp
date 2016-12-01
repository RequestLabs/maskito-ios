// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "Application.h"

void main()
{
    TheApplication = new Application();
    if (TheApplication->Create(64, 64, 512, 512, D3D_FEATURE_LEVEL_11_0, 0))
    {
        TheApplication->Run();
    }
    delete TheApplication;
}
