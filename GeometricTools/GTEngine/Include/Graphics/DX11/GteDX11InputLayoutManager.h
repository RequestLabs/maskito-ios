// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <LowLevel/GteThreadSafeMap.h>
#include <Graphics/DX11/GteDX11InputLayout.h>

namespace gte
{

class GTE_IMPEXP DX11InputLayoutManager
{
public:
    // Construction and destruction.
    ~DX11InputLayoutManager();
    DX11InputLayoutManager();

    // Management functions.  The Unbind(vbuffer) removes all pairs that
    // involve vbuffer.  The Unbind(vshader) removes all pairs that involve
    // vshader.
    DX11InputLayout* Bind(ID3D11Device* device, VertexBuffer const* vbuffer,
        Shader const* vshader);
    bool Unbind(VertexBuffer const* vbuffer, Shader const* vshader);
    bool Unbind(VertexBuffer const* vbuffer);
    bool Unbind(Shader const* vshader);
    void UnbindAll();
    DX11InputLayout* Get(VertexBuffer const* vbuffer, Shader const* vshader)
        const;
    bool HasElements() const;

private:
    typedef std::pair<VertexBuffer const*, Shader const*> VBSPair;

    class LayoutMap : public ThreadSafeMap<VBSPair, DX11InputLayout*>
    {
    public:
        virtual ~LayoutMap();
        LayoutMap();

        void GatherMatch(VertexBuffer const* vbuffer,
            std::vector<VBSPair>& matches);

        void GatherMatch(Shader const* vshader,
            std::vector<VBSPair>& matches);
    };

    LayoutMap mMap;
};

}
