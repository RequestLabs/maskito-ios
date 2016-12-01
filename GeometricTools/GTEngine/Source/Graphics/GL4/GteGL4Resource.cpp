// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <LowLevel/GteLogger.h>
#include <Graphics/GL4/GteGL4Resource.h>
using namespace gte;


GL4Resource::~GL4Resource()
{
}

GL4Resource::GL4Resource(Resource const* gtResource)
    :
    GL4GraphicsObject(gtResource)
{
}

void* GL4Resource::MapForWrite(GLenum target)
{
    glBindBuffer(target, mGLHandle);
    GLvoid* mapped = glMapBuffer(target, GL_WRITE_ONLY);
    glBindBuffer(target, 0);
    return mapped;
}

void GL4Resource::Unmap(GLenum target)
{
    glBindBuffer(target, mGLHandle);
    glUnmapBuffer(target);
    glBindBuffer(target, 0);
}

bool GL4Resource::PreparedForCopy(GLenum access) const
{
    // TODO: DX11 requires a staging resource when copying between CPU and
    // GPU.  Does OpenGL hide the double hop?

    // Verify existence of objects.
    if (mGLHandle == 0)
    {
        LogError("Resource does not exist.");
        return false;
    }

    // Verify the copy type.  TODO: Change the Resource::CopyType names to
    // be COPY_CPU_TO_GPU and COPY_GPU_TO_CPU.
    Resource::CopyType copyType = GetResource()->GetCopyType();
    if (copyType == Resource::COPY_CPU_TO_STAGING)  // CPU -> GPU
    {
        if (access == GL_WRITE_ONLY)
        {
            return true;
        }
    }
    else if (copyType == Resource::COPY_STAGING_TO_CPU)  // GPU -> CPU
    {
        if (access == GL_READ_ONLY)
        {
            return true;
        }
    }
    else if (copyType == Resource::COPY_BIDIRECTIONAL)
    {
        if (access == GL_READ_WRITE)
        {
            return true;
        }
    }

    LogError("Resource has incorrect copy type.");
    return false;
}

