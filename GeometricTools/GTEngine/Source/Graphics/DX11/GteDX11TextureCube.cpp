// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Graphics/DX11/GteDX11TextureCube.h>
using namespace gte;


DX11TextureCube::~DX11TextureCube()
{
}

DX11TextureCube::DX11TextureCube(ID3D11Device* device,
    TextureCube const* textureCube)
    :
    DX11TextureArray(textureCube)
{
    // Specify the texture description.  TODO: Support texture cube RTs?
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = textureCube->GetLength();
    desc.Height = textureCube->GetLength();
    desc.MipLevels = textureCube->GetNumLevels();
    desc.ArraySize = textureCube->GetNumItems();
    desc.Format = static_cast<DXGI_FORMAT>(textureCube->GetFormat());
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    Resource::Usage usage = textureCube->GetUsage();
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
    else  // usage == Resource::SHADER_OUTPUT
    {
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
    }

    if (textureCube->WantAutogenerateMipmaps())
    {
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
        desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
    }

    // Create the texture.
    ID3D11Texture2D* dxTexture = nullptr;
    HRESULT hr;
    if (textureCube->GetData())
    {
        unsigned int const numSubresources =
            textureCube->GetNumSubresources();
        std::vector<D3D11_SUBRESOURCE_DATA> data(numSubresources);
        for (unsigned int index = 0; index < numSubresources; ++index)
        {
            auto sr = textureCube->GetSubresource(index);
            data[index].pSysMem = sr.data;
            data[index].SysMemPitch = sr.rowPitch;
            data[index].SysMemSlicePitch = 0;
        }
        hr = device->CreateTexture2D(&desc, &data[0], &dxTexture);
    }
    else
    {
        hr = device->CreateTexture2D(&desc, nullptr, &dxTexture);
    }
    CHECK_HR_RETURN_VOID("Failed to create texture");
    mDXObject = dxTexture;

    // Create views of the texture.
    CreateSRView(device, desc);
    if (textureCube->GetUsage() == Resource::SHADER_OUTPUT)
    {
        CreateUAView(device, desc);
    }

    // Create a staging texture if requested.
    if (textureCube->GetCopyType() != Resource::COPY_NONE)
    {
        CreateStaging(device, desc);
    }
}

DX11GraphicsObject* DX11TextureCube::Create (ID3D11Device* device,
    GraphicsObject const* object)
{
    if (object->GetType() == GT_TEXTURE_CUBE)
    {
        return new DX11TextureCube(device,
            static_cast<TextureCube const*>(object));
    }

    LogError("Invalid object type.");
    return nullptr;
}

TextureCube* DX11TextureCube::GetTexture() const
{
    return static_cast<TextureCube*>(mGTObject);
}

ID3D11Texture2D* DX11TextureCube::GetDXTexture() const
{
    return static_cast<ID3D11Texture2D*>(mDXObject);
}

void DX11TextureCube::CreateStaging(ID3D11Device* device,
    D3D11_TEXTURE2D_DESC const& tx)
{
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = tx.Width;
    desc.Height = tx.Height;
    desc.MipLevels = tx.MipLevels;
    desc.ArraySize = tx.ArraySize;
    desc.Format = tx.Format;
    desc.SampleDesc.Count = tx.SampleDesc.Count;
    desc.SampleDesc.Quality = tx.SampleDesc.Quality;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = D3D11_BIND_NONE;
    desc.CPUAccessFlags = msStagingAccess[GetTexture()->GetCopyType()];
    desc.MiscFlags = D3D11_RESOURCE_MISC_NONE;

    HRESULT hr = device->CreateTexture2D(&desc, nullptr,
        reinterpret_cast<ID3D11Texture2D**>(&mStaging));
    CHECK_HR_RETURN_NONE("Failed to create staging texture");
}

void DX11TextureCube::CreateSRView(ID3D11Device* device,
    D3D11_TEXTURE2D_DESC const& tx)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    desc.Format = tx.Format;
    desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    desc.TextureCube.MostDetailedMip = 0;
    desc.TextureCube.MipLevels = tx.MipLevels;
    HRESULT hr = device->CreateShaderResourceView(GetDXTexture(), &desc,
        &mSRView);
    CHECK_HR_RETURN_NONE("Failed to create shader resource view");
}

void DX11TextureCube::CreateUAView(ID3D11Device* device,
    D3D11_TEXTURE2D_DESC const& tx)
{
    D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
    desc.Format = tx.Format;
    desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
    desc.Texture2DArray.MipSlice = 0;
    desc.Texture2DArray.FirstArraySlice = 0;
    desc.Texture2DArray.ArraySize = tx.ArraySize;
    HRESULT hr = device->CreateUnorderedAccessView(GetDXTexture(), &desc,
        &mUAView);
    CHECK_HR_RETURN_NONE("Failed to create unordered access view");
}

