// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteGraphicsObject.h>
#include <Graphics/GL4/GteOpenGL.h>

namespace gte
{

class GTE_IMPEXP GL4GraphicsObject
{
public:
    // Abstract base class.
    virtual ~GL4GraphicsObject();
protected:
    GL4GraphicsObject(GraphicsObject const* gtObject);

public:
    // Member access.
    inline GraphicsObject* GetGraphicsObject() const;
    inline GLuint GetGLHandle() const;

    // Support for debugging.
    void SetName(std::string const& name);
    inline std::string const& GetName() const;

protected:
    GraphicsObject* mGTObject;
    GLuint mGLHandle;
    std::string mName;
};


inline GraphicsObject* GL4GraphicsObject::GetGraphicsObject() const
{
    return mGTObject;
}

inline GLuint GL4GraphicsObject::GetGLHandle() const
{
    return mGLHandle;
}

inline std::string const& GL4GraphicsObject::GetName() const
{
    return mName;
}


}
