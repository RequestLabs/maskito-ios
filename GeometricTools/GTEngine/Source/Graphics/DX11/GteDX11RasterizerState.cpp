// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Graphics/DX11/GteDX11RasterizerState.h>
using namespace gte;


DX11RasterizerState::~DX11RasterizerState()
{
}

DX11RasterizerState::DX11RasterizerState(ID3D11Device* device,
    RasterizerState const* rasterizerState)
    :
    DX11DrawingState(rasterizerState)
{
    // Specify the rasterizer state description.
    D3D11_RASTERIZER_DESC desc;
    desc.FillMode = msFillMode[rasterizerState->fillMode];
    desc.CullMode = msCullMode[rasterizerState->cullMode];
    desc.FrontCounterClockwise = (rasterizerState->frontCCW ? TRUE : FALSE);
    desc.DepthBias = rasterizerState->depthBias;
    desc.DepthBiasClamp = rasterizerState->depthBiasClamp;
    desc.SlopeScaledDepthBias = rasterizerState->slopeScaledDepthBias;
    desc.DepthClipEnable = (rasterizerState->enableDepthClip ? TRUE : FALSE);
    desc.ScissorEnable = (rasterizerState->enableScissor ? TRUE : FALSE);
    desc.MultisampleEnable = 
        (rasterizerState->enableMultisample ? TRUE : FALSE);
    desc.AntialiasedLineEnable =
        (rasterizerState->enableAntialiasedLine ? TRUE : FALSE);

    // Create the rasterizer state.
    ID3D11RasterizerState* state = nullptr;
    HRESULT hr = device->CreateRasterizerState(&desc, &state);
    CHECK_HR_RETURN_NONE("Failed to create rasterizer state");
    mDXObject = state;
}

DX11GraphicsObject* DX11RasterizerState::Create(ID3D11Device* device,
    GraphicsObject const* object)
{
    if (object->GetType() == GT_RASTERIZER_STATE)
    {
        return new DX11RasterizerState(device,
            static_cast<RasterizerState const*>(object));
    }

    LogError("Invalid object type.");
    return nullptr;
}

RasterizerState* DX11RasterizerState::GetRasterizerState()
{
    return static_cast<RasterizerState*>(mGTObject);
}

ID3D11RasterizerState* DX11RasterizerState::GetDXRasterizerState()
{
    return static_cast<ID3D11RasterizerState*>(mDXObject);
}

void DX11RasterizerState::Enable(ID3D11DeviceContext* context)
{
    context->RSSetState(GetDXRasterizerState());
}


D3D11_FILL_MODE const DX11RasterizerState::msFillMode[] =
{
    D3D11_FILL_SOLID,
    D3D11_FILL_WIREFRAME
};

D3D11_CULL_MODE const DX11RasterizerState::msCullMode[] =
{
    D3D11_CULL_NONE,
    D3D11_CULL_FRONT,
    D3D11_CULL_BACK
};
