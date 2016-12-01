// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteConstantBuffer.h>
#include <Graphics/GL4/GteGL4Buffer.h>

namespace gte
{

class GTE_IMPEXP GL4ConstantBuffer : public GL4Buffer
{
public:
    // Construction and destruction.
    virtual ~GL4ConstantBuffer();
    GL4ConstantBuffer(ConstantBuffer const* cbuffer);
    static GL4GraphicsObject* Create(GraphicsObject const* object);

    // Member access.
    inline ConstantBuffer* GetConstantBuffer() const;
};


inline ConstantBuffer* GL4ConstantBuffer::GetConstantBuffer() const
{
    return static_cast<ConstantBuffer*>(mGTObject);
}


}
