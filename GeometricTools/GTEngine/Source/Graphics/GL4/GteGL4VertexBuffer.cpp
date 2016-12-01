// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <LowLevel/GteLogger.h>
#include <Graphics/GL4/GteGL4VertexBuffer.h>
using namespace gte;


GL4VertexBuffer::~GL4VertexBuffer()
{
    glDeleteBuffers(1, &mGLHandle);
}

GL4VertexBuffer::GL4VertexBuffer(VertexBuffer const* vbuffer)
    :
    GL4Buffer(vbuffer, GL_ARRAY_BUFFER)
{
}

GL4GraphicsObject* GL4VertexBuffer::Create(GraphicsObject const* object)
{
    if (object->GetType() == GT_VERTEX_BUFFER)
    {
        return new GL4VertexBuffer(static_cast<VertexBuffer const*>(object));
    }

    LogError("Invalid object type.");
    return nullptr;
}

