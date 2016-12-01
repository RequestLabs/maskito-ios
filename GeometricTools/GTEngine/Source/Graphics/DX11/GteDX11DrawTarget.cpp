// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Graphics/DX11/GteDX11DrawTarget.h>
using namespace gte;


DX11DrawTarget::~DX11DrawTarget ()
{
}

DX11DrawTarget::DX11DrawTarget(DrawTarget const* target,
    std::vector<DX11TextureRT*>& rtTextures, DX11TextureDS* dsTexture)
    :
    mTarget(target),
    mRTTextures(rtTextures),
    mDSTexture(dsTexture),
    mRTViews(target->GetNumTargets()),
    mDSView(nullptr),
    mSaveRTViews(target->GetNumTargets()),
    mSaveDSView(nullptr)
{
    unsigned int const numTargets = mTarget->GetNumTargets();
    for (unsigned int i = 0; i < numTargets; ++i)
    {
        mRTViews[i] = mRTTextures[i]->GetRTView();
        mSaveRTViews[i] = nullptr;
#if defined(GTE_USE_NAMED_DX11_OBJECTS)
        mRTTextures[i]->SetName(target->GetRTTexture(i)->GetName());
#endif
    }

    if (mDSTexture)
    {
        mDSView = mDSTexture->GetDSView();
#if defined(GTE_USE_NAMED_DX11_OBJECTS)
        mDSTexture->SetName(target->GetDSTexture()->GetName());
#endif
    }
}

DX11TextureRT* DX11DrawTarget::GetRTTexture(unsigned int i) const
{
    return mRTTextures[i];
}

DX11TextureDS* DX11DrawTarget::GetDSTexture() const
{
    return mDSTexture;
}

void DX11DrawTarget::Enable (ID3D11DeviceContext* context)
{
    UINT numViewports = 1;
    context->RSGetViewports(&numViewports, &mSaveViewport);

    UINT const numTargets = (UINT)mTarget->GetNumTargets();
    context->OMGetRenderTargets(numTargets, &mSaveRTViews[0], &mSaveDSView);

    D3D11_VIEWPORT viewport;
    viewport.Width = static_cast<float>(mTarget->GetWidth());
    viewport.Height = static_cast<float>(mTarget->GetHeight());
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    context->RSSetViewports(1, &viewport);

    context->OMSetRenderTargets(numTargets, &mRTViews[0], mDSView);
}

void DX11DrawTarget::Disable (ID3D11DeviceContext* context)
{
    context->RSSetViewports(1, &mSaveViewport);

    UINT const numTargets = (UINT)mTarget->GetNumTargets();
    context->OMSetRenderTargets(numTargets, &mSaveRTViews[0], mSaveDSView);
    for (unsigned int i = 0; i < numTargets; ++i)
    {
        if (mSaveRTViews[i])
        {
            SafeRelease(mSaveRTViews[i]);
        }
    }
    if (mSaveDSView)
    {
        SafeRelease(mSaveDSView);
    }
}

