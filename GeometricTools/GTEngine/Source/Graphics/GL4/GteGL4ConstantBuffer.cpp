// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <LowLevel/GteLogger.h>
#include <Graphics/GL4/GteGL4ConstantBuffer.h>
using namespace gte;


GL4ConstantBuffer::~GL4ConstantBuffer()
{
}

GL4ConstantBuffer::GL4ConstantBuffer(ConstantBuffer const* cbuffer)
    :
    GL4Buffer(cbuffer, GL_UNIFORM_BUFFER)
{
}

GL4GraphicsObject* GL4ConstantBuffer::Create(GraphicsObject const* object)
{
    if (object->GetType() == GT_CONSTANT_BUFFER)
    {
        return new GL4ConstantBuffer(
            static_cast<ConstantBuffer const*>(object));
    }

    LogError("Invalid object type.");
    return nullptr;
}

