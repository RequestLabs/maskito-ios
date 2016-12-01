// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteTextureArray.h>

namespace gte
{

class GTE_IMPEXP TextureCubeArray : public TextureArray
{
public:
    // Construction.  Cube maps must be square; the 'length' parameter is the
    // shared value for width and height of a face.  The 'numCubes' is the
    // number of 6-tuples of cube maps.
    TextureCubeArray(unsigned int numCubes, DFType format,
        unsigned int length, bool hasMipmaps = false,
        bool createStorage = true);

    // Member access.
    unsigned int GetNumCubes() const;

    // The texture width and height are the same value.
    unsigned int GetLength() const;

private:
    unsigned int mNumCubes;
};

}
