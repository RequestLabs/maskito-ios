// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Graphics/GteControlledObject.h>
using namespace gte;


ControlledObject::~ControlledObject()
{
}

ControlledObject::ControlledObject()
{
}

void ControlledObject::AttachController(Controller* controller)
{
    if (controller)
    {
        // Test whether the controller is already in the list.
        for (auto const& element : mControllers)
        {
            if (element == controller)
            {
                // The controller is in the list, so nothing to do.
                return;
            }
        }

        // Bind the controller to the object.
        controller->SetObject(this);

        // The controller is not in the current list, so add it.
        mControllers.push_back(controller);
    }
}

void ControlledObject::DetachController(Controller* controller)
{
    for (auto const& element : mControllers)
    {
        if (element == controller)
        {
            // Unbind the controller from the object.
            controller->SetObject(nullptr);

            // Remove the controller from the list.
            mControllers.remove(controller);
            return;
        }
    }
}

void ControlledObject::DetachAllControllers()
{
    for (auto& element : mControllers)
    {
        // Unbind the controller from the object.
        element->SetObject(nullptr);
    }
    mControllers.clear();
}

bool ControlledObject::UpdateControllers(double applicationTime)
{
    bool someoneUpdated = false;
    for (auto& element : mControllers)
    {
        if (element->Update(applicationTime))
        {
            someoneUpdated = true;
        }
    }
    return someoneUpdated;
}

