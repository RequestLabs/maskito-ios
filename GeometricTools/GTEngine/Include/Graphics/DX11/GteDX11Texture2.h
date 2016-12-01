// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteTexture2.h>
#include <Graphics/DX11/GteDX11TextureSingle.h>

namespace gte
{

class GTE_IMPEXP DX11Texture2 : public DX11TextureSingle
{
public:
    // Construction and destruction.
    virtual ~DX11Texture2();
    DX11Texture2(ID3D11Device* device, Texture2 const* texture);
    DX11Texture2(ID3D11Device* device, DX11Texture2 const* dxSharedTexture);
    static DX11GraphicsObject* Create(ID3D11Device* device,
        GraphicsObject const* object);
protected:
    // Constructors for DX11TextureRT and DX11TextureDS.
    DX11Texture2(Texture2 const* texture);

public:
    // Member access.
    Texture2* GetTexture() const;
    ID3D11Texture2D* GetDXTexture() const;

protected:
    // Support for construction.
    ID3D11Texture2D* CreateSharedDXObject(ID3D11Device* device) const;
    void CreateStaging(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx);
    void CreateSRView(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx);
    void CreateUAView(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx);
};

}
