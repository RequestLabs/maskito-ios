// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteTextureDS.h>
#include <Graphics/DX11/GteDX11Texture2.h>

namespace gte
{

class GTE_IMPEXP DX11TextureDS : public DX11Texture2
{
public:
    // Construction and destruction.
    virtual ~DX11TextureDS();
    DX11TextureDS(ID3D11Device* device, TextureDS const* texture);
    DX11TextureDS(ID3D11Device* device, DX11TextureDS const* dxSharedTexture);
    static DX11GraphicsObject* Create(ID3D11Device* device,
        GraphicsObject const* object);

    // Member access.
    ID3D11DepthStencilView* GetDSView() const;

private:
    // Support for construction.
    void CreateDSView(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx);

    ID3D11DepthStencilView* mDSView;

public:
    // Support for the DX11 debug layer; see comments in the file
    // GteDX11GraphicsObject.h about usage.
    virtual void SetName(std::string const& name);
};

}
