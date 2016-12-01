// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <LowLevel/GteThreadSafeMap.h>
#include <Graphics/GL4/GteGL4InputLayout.h>

namespace gte
{

class GTE_IMPEXP GL4InputLayoutManager
{
public:
    // Construction and destruction.
    ~GL4InputLayoutManager();
    GL4InputLayoutManager();

    // Management functions.  The Unbind(vbuffer) removes all layouts that
    // involve vbuffer.
    GL4InputLayout* Bind(GLuint programHandle, GLuint vbufferHandle,
        VertexBuffer const* vbuffer);
    bool Unbind(VertexBuffer const* vbuffer, GLuint programHandle);
    bool Unbind(VertexBuffer const* vbuffer);
    bool Unbind(GLuint programHandle);
    void UnbindAll();
    GL4InputLayout* Get(VertexBuffer const* vbuffer, GLuint programHandle)
        const;
    bool HasElements() const;

private:
    typedef std::pair<VertexBuffer const*, GLuint> VBPPair;

    class LayoutMap : public ThreadSafeMap<VBPPair, GL4InputLayout*>
    {
    public:
        virtual ~LayoutMap();
        LayoutMap();

        void GatherMatch(VertexBuffer const* vbuffer,
            std::vector<VBPPair>& matches);

        void GatherMatch(GLuint programHandle,
            std::vector<VBPPair>& matches);
    };

    LayoutMap mMap;
};

}
