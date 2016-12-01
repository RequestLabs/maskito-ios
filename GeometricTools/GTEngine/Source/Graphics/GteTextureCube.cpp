// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Graphics/GteTextureCube.h>
using namespace gte;


TextureCube::TextureCube(DFType format, unsigned int length, bool hasMipmaps,
    bool createStorage)
    :
    TextureArray(6, format, 2, length, length, 1, hasMipmaps, createStorage)
{
    mType = GT_TEXTURE_CUBE;
}

unsigned int TextureCube::GetLength() const
{
    return TextureArray::GetDimension(0);
}

