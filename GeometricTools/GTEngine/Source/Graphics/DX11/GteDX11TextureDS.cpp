// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Graphics/DX11/GteDX11TextureDS.h>
using namespace gte;


DX11TextureDS::~DX11TextureDS()
{
    FinalRelease(mDSView);
}

DX11TextureDS::DX11TextureDS(ID3D11Device* device, TextureDS const* texture)
    :
    DX11Texture2(texture),
    mDSView(nullptr)
{
    // Specify the texture description.
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = texture->GetWidth();
    desc.Height = texture->GetHeight();
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = static_cast<DXGI_FORMAT>(texture->GetFormat());
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
    desc.MiscFlags = (texture->IsShared() ?
        D3D11_RESOURCE_MISC_SHARED : D3D11_RESOURCE_MISC_NONE);

    // Create the texture.  Depth-stencil textures are not initialized by
    // system memory data.
    ID3D11Texture2D* dxTexture = nullptr;
    HRESULT hr = device->CreateTexture2D(&desc, nullptr, &dxTexture);
    CHECK_HR_RETURN_VOID("Failed to create depth-stencil texture");
    mDXObject = dxTexture;

    // Create a view of the texture.
    CreateDSView(device, desc);

    // Create a staging texture if requested.
    if (texture->GetCopyType() != Resource::COPY_NONE)
    {
        CreateStaging(device, desc);
    }
}

DX11TextureDS::DX11TextureDS(ID3D11Device* device,
    DX11TextureDS const* dxSharedTexture)
    :
    DX11Texture2(dxSharedTexture->GetTexture()),
    mDSView(nullptr)
{
    ID3D11Texture2D* dxShared = dxSharedTexture->CreateSharedDXObject(device);
    mDXObject = dxShared;
    D3D11_TEXTURE2D_DESC desc;
    dxShared->GetDesc(&desc);
    CreateDSView(device, desc);
}

DX11GraphicsObject* DX11TextureDS::Create(ID3D11Device* device,
    GraphicsObject const* object)
{
    if (object->GetType() == GT_TEXTURE_DS)
    {
        return new DX11TextureDS(device,
            static_cast<TextureDS const*>(object));
    }

    LogError("Invalid object type.");
    return nullptr;
}

ID3D11DepthStencilView* DX11TextureDS::GetDSView() const
{
    return mDSView;
}

void DX11TextureDS::CreateDSView(ID3D11Device* device,
    D3D11_TEXTURE2D_DESC const& tx)
{
    D3D11_DEPTH_STENCIL_VIEW_DESC desc;
    desc.Format = tx.Format;
    desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    desc.Flags = 0;
    desc.Texture2D.MipSlice = 0;
    HRESULT hr = device->CreateDepthStencilView(GetDXTexture(), &desc,
        &mDSView);
    CHECK_HR_RETURN_NONE("Failed to create depth-stencil view");
}

void DX11TextureDS::SetName(std::string const& name)
{
    DX11Texture2::SetName(name);
    HRESULT hr = SetPrivateName(mDSView, name);
    CHECK_HR_RETURN_NONE("Failed to set private name");
}

