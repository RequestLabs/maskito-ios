// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <LowLevel/GteLogger.h>
#include <Graphics/GL4/GteGL4Buffer.h>
using namespace gte;


GL4Buffer::~GL4Buffer()
{
}

GL4Buffer::GL4Buffer(Buffer const* buffer, GLenum type)
    :
    GL4Resource(buffer),
    mType(type)
{
    glGenBuffers(1, &mGLHandle);

    Resource::Usage usage = buffer->GetUsage();
    GLenum glUsage;
    if (usage == Resource::IMMUTABLE)
    {
        glUsage = GL_STATIC_DRAW;
    }
    else if (usage == Resource::DYNAMIC_UPDATE)
    {
        glUsage = GL_DYNAMIC_DRAW;
    }
    else  // usage == Resource::SHADER_OUTPUT
    {
        // TODO: In GLSL, is it possible to write to a buffer other than a
        // vertex buffer?
        if (mType == GL_ARRAY_BUFFER)
        {
            glUsage = GL_STREAM_DRAW;
        }
        else
        {
            LogWarning("Can a non-vertex buffer be a shader output?");
            glUsage = GL_STATIC_DRAW;
        }
    }

    glBindBuffer(mType, mGLHandle);
    glBufferData(mType, buffer->GetNumBytes(), buffer->GetData(), glUsage);
    glBindBuffer(mType, 0);
}

bool GL4Buffer::Update()
{
    Buffer* buffer = GetBuffer();
    if (buffer->GetUsage() != Resource::DYNAMIC_UPDATE)
    {
        LogWarning("Buffer usage is not DYNAMIC_UPDATE.");
        return false;
    }

    UINT numActiveBytes = buffer->GetNumActiveBytes();
    if (numActiveBytes > 0)
    {
        // Copy from CPU memory to GPU memory.
        unsigned int offset = buffer->GetOffset();
        void const* data = buffer->GetData() + offset;
        glBindBuffer(mType, mGLHandle);
        glBufferSubData(mType, 0, numActiveBytes, data);
        glBindBuffer(mType, 0);
    }
    else
    {
        LogInformation("Buffer has zero active bytes.");
    }
    return true;
}

bool GL4Buffer::CopyCpuToGpu()
{
    if (!PreparedForCopy(GL_WRITE_ONLY))
    {
        return false;
    }

    Buffer* buffer = GetBuffer();
    UINT numActiveBytes = buffer->GetNumActiveBytes();
    if (numActiveBytes > 0)
    {
        // Copy from CPU memory to GPU memory.
        glBindBuffer(mType, mGLHandle);
        glBufferSubData(mType, buffer->GetOffset(), numActiveBytes,
            buffer->GetData());
        glBindBuffer(mType, 0);
    }
    else
    {
        LogInformation("Buffer has zero active bytes.");
    }
    return true;
}

bool GL4Buffer::CopyGpuToCpu()
{
    if (!PreparedForCopy(GL_READ_ONLY))
    {
        return false;
    }

    Buffer* buffer = GetBuffer();
    UINT numActiveBytes = buffer->GetNumActiveBytes();
    if (numActiveBytes > 0)
    {
        // Copy from GPU memory to CPU memory.
        glBindBuffer(mType, mGLHandle);
        glGetBufferSubData(mType, buffer->GetOffset(),
            numActiveBytes, buffer->GetData());
        glBindBuffer(mType, 0);
    }
    else
    {
        LogInformation("Buffer has zero active bytes.");
    }
    return true;
}

