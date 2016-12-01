// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Graphics/GteTextureBuffer.h>
using namespace gte;


TextureBuffer::TextureBuffer(DFType format, unsigned int numElements,
    bool allowDynamicUpdate)
    :
    Buffer(numElements, DataFormat::GetNumBytesPerStruct(format), true),
    mFormat(format)
{
    mType = GT_TEXTURE_BUFFER;
    mUsage = (allowDynamicUpdate ? DYNAMIC_UPDATE : IMMUTABLE);
    memset(mData, 0, mNumBytes);
}

DFType TextureBuffer::GetFormat() const
{
    return mFormat;
}

bool TextureBuffer::HasMember(std::string const& name) const
{
    auto iter = std::find_if(mLayout.begin(), mLayout.end(),
        [&name](MemberLayout const& item){ return name == item.name; });
    return iter != mLayout.end();
}

