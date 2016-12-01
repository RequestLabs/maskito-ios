// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Graphics/DX11/GteDX11ConstantBuffer.h>
using namespace gte;


DX11ConstantBuffer::~DX11ConstantBuffer()
{
}

DX11ConstantBuffer::DX11ConstantBuffer(ID3D11Device* device,
    ConstantBuffer const* cbuffer)
    :
    DX11Buffer(cbuffer)
{
    // Specify the buffer description.
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = cbuffer->GetNumBytes();
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.MiscFlags = D3D11_RESOURCE_MISC_NONE;
    desc.StructureByteStride = 0;
    Resource::Usage usage = cbuffer->GetUsage();
    if (usage == Resource::IMMUTABLE)
    {
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
    }
    else if (usage == Resource::DYNAMIC_UPDATE)
    {
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    }
    else
    {
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
    }

    // Create the buffer.
    ID3D11Buffer* buffer = nullptr;
    HRESULT hr;
    if (cbuffer->GetData())
    {
        D3D11_SUBRESOURCE_DATA data;
        data.pSysMem = cbuffer->GetData();
        data.SysMemPitch = 0;
        data.SysMemSlicePitch = 0;
        hr = device->CreateBuffer(&desc, &data, &buffer);
    }
    else
    {
        hr = device->CreateBuffer(&desc, nullptr, &buffer);
    }
    CHECK_HR_RETURN_NONE("Failed to create constant buffer");
    mDXObject = buffer;

    // Create a staging buffer if requested.
    if (cbuffer->GetCopyType() != Resource::COPY_NONE)
    {
        CreateStaging(device, desc);
    }
}

DX11GraphicsObject* DX11ConstantBuffer::Create(ID3D11Device* device,
    GraphicsObject const* object)
{
    if (object->GetType() == GT_CONSTANT_BUFFER)
    {
        return new DX11ConstantBuffer(device,
            static_cast<ConstantBuffer const*>(object));
    }

    LogError("Invalid object type.");
    return nullptr;
}

