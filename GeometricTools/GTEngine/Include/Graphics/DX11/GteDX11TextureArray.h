// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteTextureArray.h>
#include <Graphics/DX11/GteDX11Texture.h>

namespace gte
{

class GTE_IMPEXP DX11TextureArray : public DX11Texture
{
public:
    // Abstract base class, a shim to distinguish between single textures and
    // texture arrays.
    virtual ~DX11TextureArray();
protected:
    // No public construction.  Derived classes use this constructor.
    DX11TextureArray(TextureArray const* gtTextureArray);

public:
    // Member access.
    TextureArray* GetTextureArray() const;
};

}
