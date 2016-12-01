// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteTextureBuffer.h>
#include <Graphics/DX11/GteDX11Buffer.h>

namespace gte
{

class GTE_IMPEXP DX11TextureBuffer : public DX11Buffer
{
public:
    // Construction and destruction.
    virtual ~DX11TextureBuffer();
    DX11TextureBuffer(ID3D11Device* device, TextureBuffer const* tbuffer);
    static DX11GraphicsObject* Create(ID3D11Device* device,
        GraphicsObject const* object);

    // Member access.
    TextureBuffer* GetTextureBuffer() const;
    ID3D11ShaderResourceView* GetSRView() const;

private:
    // Support for construction.
    void CreateSRView(ID3D11Device* device);

    ID3D11ShaderResourceView* mSRView;

public:
    // Support for the DX11 debug layer; see comments in the file
    // GteDX11GraphicsObject.h about usage.
    virtual void SetName(std::string const& name);
};

}
