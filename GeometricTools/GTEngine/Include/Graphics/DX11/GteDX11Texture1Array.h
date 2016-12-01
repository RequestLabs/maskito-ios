// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteTexture1Array.h>
#include <Graphics/DX11/GteDX11TextureArray.h>

namespace gte
{

class GTE_IMPEXP DX11Texture1Array : public DX11TextureArray
{
public:
    // Construction and destruction.
    virtual ~DX11Texture1Array();
    DX11Texture1Array(ID3D11Device* device,
        Texture1Array const* textureArray);
    static DX11GraphicsObject* Create(ID3D11Device* device,
        GraphicsObject const* object);

    // Member access.
    Texture1Array* GetTextureArray() const;
    ID3D11Texture1D* GetDXTextureArray() const;

private:
    // Support for construction.
    void CreateStaging(ID3D11Device* device, D3D11_TEXTURE1D_DESC const& tx);
    void CreateSRView(ID3D11Device* device, D3D11_TEXTURE1D_DESC const& tx);
    void CreateUAView(ID3D11Device* device, D3D11_TEXTURE1D_DESC const& tx);
};

}
