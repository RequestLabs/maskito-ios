// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteController.h>
#include <Graphics/GteTransform.h>

namespace gte
{

class GTE_IMPEXP TransformController : public Controller
{
public:
    TransformController(Transform const& localTransform);

    // Member access.
    inline void SetTransform(Transform const& localTransform);
    inline Transform const& GetTransform() const;

    // The animation update.  The application time is in milliseconds.
    // The update simply copies mLocalTransform to the Spatial mObject's
    // LocalTransform.  In this sense, TransformController represents a
    // transform that is constant for all time.
    virtual bool Update(double applicationTime);

protected:
    Transform mLocalTransform;
};


inline void TransformController::SetTransform(Transform const& localTransform)
{
    mLocalTransform = localTransform;
}

inline Transform const& TransformController::GetTransform() const
{
    return mLocalTransform;
}


}
