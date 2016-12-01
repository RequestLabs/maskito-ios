// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteDrawingState.h>
#include <Graphics/GL4/GteGL4GraphicsObject.h>

namespace gte
{

class GTE_IMPEXP GL4DrawingState : public GL4GraphicsObject
{
public:
    // Construction and destruction.
    virtual ~GL4DrawingState();
protected:
    // No public construction.
    GL4DrawingState(DrawingState const* gtState);
};

}
