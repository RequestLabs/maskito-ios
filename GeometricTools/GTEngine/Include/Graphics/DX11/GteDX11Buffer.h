// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteBuffer.h>
#include <Graphics/DX11/GteDX11Resource.h>

namespace gte
{

class GTE_IMPEXP DX11Buffer : public DX11Resource
{
public:
    // Abstract base class.
    virtual ~DX11Buffer();
protected:
    DX11Buffer(Buffer const* buffer);

public:
    // Member access.
    Buffer* GetBuffer() const;
    ID3D11Buffer* GetDXBuffer() const;

    // Copy data from CPU to GPU via mapped memory.  Buffers use only
    // subresource 0, so the subresource index (sri) is not exposed.
    virtual bool Update(ID3D11DeviceContext* context);
    virtual bool CopyCpuToGpu(ID3D11DeviceContext* context);
    virtual bool CopyGpuToCpu(ID3D11DeviceContext* context);

private:
    // Buffers use only subresource 0, so these overrides are stubbed out.
    virtual bool Update(ID3D11DeviceContext* context, unsigned int sri);
    virtual bool CopyCpuToGpu(ID3D11DeviceContext* context, unsigned int sri);
    virtual bool CopyGpuToCpu(ID3D11DeviceContext* context, unsigned int sri);

protected:
    // Support for creating staging buffers.
    void CreateStaging(ID3D11Device* device, D3D11_BUFFER_DESC const& bf);
};

}
