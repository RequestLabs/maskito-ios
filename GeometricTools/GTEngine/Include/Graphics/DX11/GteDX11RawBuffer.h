// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteRawBuffer.h>
#include <Graphics/DX11/GteDX11Buffer.h>

namespace gte
{

class GTE_IMPEXP DX11RawBuffer : public DX11Buffer
{
public:
    // Construction and destruction.
    virtual ~DX11RawBuffer();
    DX11RawBuffer(ID3D11Device* device, RawBuffer const* rbuffer);
    static DX11GraphicsObject* Create(ID3D11Device* device,
        GraphicsObject const* object);

    // Member access.
    RawBuffer* GetRawBuffer() const;
    ID3D11ShaderResourceView* GetSRView() const;
    ID3D11UnorderedAccessView* GetUAView() const;

private:
    // Support for construction.
    void CreateSRView(ID3D11Device* device);
    void CreateUAView(ID3D11Device* device);

    ID3D11ShaderResourceView* mSRView;
    ID3D11UnorderedAccessView* mUAView;

public:
    // Support for the DX11 debug layer; see comments in the file
    // GteDX11GraphicsObject.h about usage.
    virtual void SetName(std::string const& name);
};

}
