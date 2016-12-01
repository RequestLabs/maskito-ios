// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <LowLevel/GteLogger.h>
#include <Graphics/GL4/GteGL4InputLayoutManager.h>
using namespace gte;


GL4InputLayoutManager::~GL4InputLayoutManager()
{
    if (mMap.HasElements())
    {
        LogWarning("Input layout map is not empty on destruction.");
        UnbindAll();
    }
}

GL4InputLayoutManager::GL4InputLayoutManager()
{
}

GL4InputLayout* GL4InputLayoutManager::Bind(GLuint programHandle,
    GLuint vbufferHandle, VertexBuffer const* vbuffer)
{
    GL4InputLayout* layout = nullptr;
    if (programHandle)
    {
        if (vbuffer)
        {
            if (!mMap.Get(std::make_pair(vbuffer, programHandle), layout))
            {
                layout = new GL4InputLayout(programHandle, vbufferHandle,
                    vbuffer);
                mMap.Insert(std::make_pair(vbuffer, programHandle), layout);
            }
        }
        // else: A null vertex buffer is passed when an effect wants to
        // bypass the input assembler.
    }
    else
    {
        LogError("Program must exist.");
    }
    return layout;
}

bool GL4InputLayoutManager::Unbind(VertexBuffer const* vbuffer,
    GLuint programHandle)
{
    if (programHandle)
    {
        if (vbuffer)
        {
            GL4InputLayout* layout = nullptr;
            if (mMap.Remove(std::make_pair(vbuffer, programHandle), layout))
            {
                delete layout;
            }
        }
        // else:  A null vertex buffer is passed when an effect wants to
        // bypass the input assembler.
        return true;
    }
    else
    {
        LogError("Program must exist.");
        return false;
    }
}

bool GL4InputLayoutManager::Unbind(VertexBuffer const* vbuffer)
{
    if (vbuffer)
    {
        std::vector<VBPPair> matches;
        mMap.GatherMatch(vbuffer, matches);
        for (auto match : matches)
        {
            GL4InputLayout* layout = nullptr;
            if (mMap.Remove(match, layout))
            {
                delete layout;
            }
        }
        return true;
    }
    else
    {
        LogError("Vertex buffer must be nonnull.");
        return false;
    }
}

bool GL4InputLayoutManager::Unbind(GLuint programHandle)
{
    if (programHandle)
    {
        std::vector<VBPPair> matches;
        mMap.GatherMatch(programHandle, matches);
        for (auto match : matches)
        {
            GL4InputLayout* layout = nullptr;
            if (mMap.Remove(match, layout))
            {
                delete layout;
            }
        }
        return true;
    }
    else
    {
        LogError("Program must exist.");
        return false;
    }
}

void GL4InputLayoutManager::UnbindAll()
{
    std::vector<GL4InputLayout*> layouts;
    mMap.GatherAll(layouts);
    for (auto layout : layouts)
    {
        delete layout;
    }
    mMap.RemoveAll();
}

GL4InputLayout* GL4InputLayoutManager::Get(VertexBuffer const* vbuffer,
    GLuint programHandle) const
{
    GL4InputLayout* layout = nullptr;
    if (!mMap.Get(std::make_pair(vbuffer, programHandle), layout))
    {
        LogWarning("Cannot find input layout.");
    }
    return layout;
}

bool GL4InputLayoutManager::HasElements() const
{
    return mMap.HasElements();
}

GL4InputLayoutManager::LayoutMap::~LayoutMap()
{
}

GL4InputLayoutManager::LayoutMap::LayoutMap()
{
}

void GL4InputLayoutManager::LayoutMap::GatherMatch(
    VertexBuffer const* vbuffer, std::vector<VBPPair>& matches)
{
    mMutex.lock();
    {
        for (auto vbp : mMap)
        {
            if (vbuffer == vbp.first.first)
            {
                matches.push_back(vbp.first);
            }
        }
    }
    mMutex.unlock();
}

void GL4InputLayoutManager::LayoutMap::GatherMatch(GLuint programHandle,
    std::vector<VBPPair>& matches)
{
    mMutex.lock();
    {
        for (auto vbp : mMap)
        {
            if (programHandle == vbp.first.second)
            {
                matches.push_back(vbp.first);
            }
        }
    }
    mMutex.unlock();
}

