// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteTexture.h>
#include <Graphics/GL4/GteGL4Resource.h>

namespace gte
{

class GTE_IMPEXP GL4Texture : public GL4Resource
{
public:
    // Abstract base class.
    virtual ~GL4Texture();
protected:
    GL4Texture(Texture const* texture, GLenum type);

public:
    // Member access.
    inline Texture* GetTexture() const;
    inline GLenum GetType() const;

protected:
    GLenum mType;
};


inline Texture* GL4Texture::GetTexture() const
{
    return static_cast<Texture*>(mGTObject);
}

inline GLenum GL4Texture::GetType() const
{
    return mType;
}


}
