// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteViewVolume.h>
#include <Graphics/GteLighting.h>
#include <memory>

namespace gte
{

class GTE_IMPEXP Light : public ViewVolume
{
public:
    // Construction.  The defaults are listed for each member.
    Light(bool isPerspective = false);

    std::shared_ptr<Lighting> lighting;
};

}
