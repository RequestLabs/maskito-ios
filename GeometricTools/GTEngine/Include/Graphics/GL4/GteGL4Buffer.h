// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteBuffer.h>
#include <Graphics/GL4/GteGL4Resource.h>

namespace gte
{

class GTE_IMPEXP GL4Buffer : public GL4Resource
{
public:
    // Abstract base class.
    virtual ~GL4Buffer();
protected:
    GL4Buffer(Buffer const* buffer, GLenum type);

public:
    // Member access.
    inline Buffer* GetBuffer() const;
    inline GLenum GetType() const;

    // TODO: TENTATIVE INTERFACE (modify as needed).  Do we really need
    // these to be virtual?  Revisit the DX11 code and investigate why the
    // choice was made there.
    virtual bool Update();
    virtual bool CopyCpuToGpu();
    virtual bool CopyGpuToCpu();

protected:
    GLenum mType;
};


inline Buffer* GL4Buffer::GetBuffer() const
{
    return static_cast<Buffer*>(mGTObject);
}

inline GLenum GL4Buffer::GetType() const
{
    return mType;
}


}
