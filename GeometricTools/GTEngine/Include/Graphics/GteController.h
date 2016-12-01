// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngineDEF.h>
#include <string>

namespace gte
{

class ControlledObject;

class GTE_IMPEXP Controller
{
protected:
    // Abstract base class.
    Controller();
public:
    virtual ~Controller();

    // Member access.
    inline ControlledObject* GetObject() const;
    inline void SetApplicationTime(double applicationTime);
    inline double GetApplicationTime() const;

    // The animation update.  The application time is in milliseconds.
    virtual bool Update(double applicationTime);

    // Time management.  A controller may use its own time scale, and it
    // specifies how times are to be mapped to application time.
    enum GTE_IMPEXP RepeatType
    {
        RT_CLAMP,
        RT_WRAP,
        RT_CYCLE
    };

    // Member access.
    RepeatType repeat;  // default = RT_CLAMP
    double minTime;     // default = 0
    double maxTime;     // default = 0
    double phase;       // default = 0
    double frequency;   // default = 1
    bool active;        // default = true

    // Allow user-readable names for nodes in a scene graph.
    std::string name;

public:  // INTERNAL USE ONLY
    // The class ControlledObject needs to set the object during a call to
    // AttachController.  Derived classes that manage a set of controllers
    // also need to set the objects during a call to AttachController.
    virtual void SetObject(ControlledObject* object);

protected:
    // Conversion from application time units to controller time units.
    // Derived classes may use this in their update routines.
    double GetControlTime(double applicationTime);

    // The controlled object.  This is a regular pointer rather than a
    // shared pointer to avoid the reference cycle between mObject and
    // 'this'.  TODO: Replace this by std::weak_ptr.
    ControlledObject* mObject;

    // The application time in milliseconds.
    double mApplicationTime;
};

}
