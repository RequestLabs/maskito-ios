// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteRasterizerState.h>
#include <Graphics/DX11/GteDX11DrawingState.h>

namespace gte
{

class GTE_IMPEXP DX11RasterizerState : public DX11DrawingState
{
public:
    // Construction and destruction.
    virtual ~DX11RasterizerState();
    DX11RasterizerState(ID3D11Device* device,
        RasterizerState const* rasterizerState);
    static DX11GraphicsObject* Create(ID3D11Device* device,
        GraphicsObject const* object);

    // Member access.
    RasterizerState* GetRasterizerState();
    ID3D11RasterizerState* GetDXRasterizerState();

    // Enable the rasterizer state.
    void Enable(ID3D11DeviceContext* context);

private:
    // Conversions from GTEngine values to DX11 values.
    static D3D11_FILL_MODE const msFillMode[];
    static D3D11_CULL_MODE const msCullMode[];
};

}
