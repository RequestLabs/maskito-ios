// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteTextureCubeArray.h>
#include <Graphics/DX11/GteDX11TextureArray.h>

namespace gte
{

class GTE_IMPEXP DX11TextureCubeArray : public DX11TextureArray
{
public:
    // Construction and destruction.
    virtual ~DX11TextureCubeArray();
    DX11TextureCubeArray(ID3D11Device* device,
        TextureCubeArray const* textureCubeArray);
    static DX11GraphicsObject* Create(ID3D11Device* device,
        GraphicsObject const* object);

    // Member access.
    TextureCubeArray* GetTextureArray() const;
    ID3D11Texture2D* GetDXTextureArray() const;

private:
    // Support for construction.
    void CreateStaging(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx);
    void CreateSRView(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx);
    void CreateUAView(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx);
};

}
