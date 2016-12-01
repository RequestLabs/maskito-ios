// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Mathematics/GteVector4.h>

namespace gte
{

class GTE_IMPEXP Material
{
public:
    // Construction.
    Material();

    // (r,g,b,*): default (0,0,0,1)
    Vector4<float> emissive;

    // (r,g,b,*): default (0,0,0,1)
    Vector4<float> ambient;

    // (r,g,b,a): default (0,0,0,1)
    Vector4<float> diffuse;

    // (r,g,b,specularPower): default (0,0,0,1)
    Vector4<float> specular;
};

}
