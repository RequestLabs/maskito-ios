// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteStructuredBuffer.h>
#include <Graphics/DX11/GteDX11Buffer.h>

namespace gte
{

class GTE_IMPEXP DX11StructuredBuffer : public DX11Buffer
{
public:
    // Construction and destruction, including factory creation.
    virtual ~DX11StructuredBuffer();
    DX11StructuredBuffer(ID3D11Device* device,
        StructuredBuffer const* sbuffer);
    static DX11GraphicsObject* Create(ID3D11Device* device,
        GraphicsObject const* object);

public:
    // Member access.
    StructuredBuffer* GetStructuredBuffer() const;
    ID3D11ShaderResourceView* GetSRView() const;
    ID3D11UnorderedAccessView* GetUAView() const;
    ID3D11Buffer* GetCounterStagingBuffer() const;

    // Copy of data between CPU and GPU.
    virtual bool CopyGpuToCpu(ID3D11DeviceContext* context);

    // The number of active elements is read from GPU and stored in the
    // StructuredBuffer object  (of type CT_APPEND_CONSUME or CT_COUNTER).
    // The result can be accessed from this object via its member function
    // GetNumActiveElements().
    bool GetNumActiveElements(ID3D11DeviceContext* context);

private:
    // Support for construction.
    void CreateSRView(ID3D11Device* device);
    void CreateUAView(ID3D11Device* device);
    void CreateCounterStaging(ID3D11Device* device);

    ID3D11ShaderResourceView* mSRView;
    ID3D11UnorderedAccessView* mUAView;
    ID3D11Buffer* mCounterStaging;

    // Mapping from StructuredBuffer::CounterType to D3D11_BUFFER_UAV_FLAG.
    static unsigned int const msUAVFlag[];

public:
    // Support for the DX11 debug layer; see comments in the file
    // GteDX11GraphicsObject.h about usage.
    virtual void SetName(std::string const& name);
};

}
