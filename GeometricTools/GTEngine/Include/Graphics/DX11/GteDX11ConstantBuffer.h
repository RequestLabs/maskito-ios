// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteConstantBuffer.h>
#include <Graphics/DX11/GteDX11Buffer.h>

namespace gte
{

class GTE_IMPEXP DX11ConstantBuffer : public DX11Buffer
{
public:
    // Construction and destruction.
    virtual ~DX11ConstantBuffer();
    DX11ConstantBuffer(ID3D11Device* device, ConstantBuffer const* cbuffer);
    static DX11GraphicsObject* Create(ID3D11Device* device,
        GraphicsObject const* object);

    // Member access.
    ConstantBuffer* GetConstantBuffer() const;
};

}
