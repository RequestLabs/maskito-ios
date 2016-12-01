// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteDepthStencilState.h>
#include <Graphics/DX11/GteDX11DrawingState.h>

namespace gte
{

class GTE_IMPEXP DX11DepthStencilState : public DX11DrawingState
{
public:
    // Construction and destruction.
    virtual ~DX11DepthStencilState();
    DX11DepthStencilState(ID3D11Device* device,
        DepthStencilState const* depthStencilState);
    static DX11GraphicsObject* Create(ID3D11Device* device,
        GraphicsObject const* object);

    // Member access.
    DepthStencilState* GetDepthStencilState();
    ID3D11DepthStencilState* GetDXDepthStencilState();

    // Enable the depth-stencil state.
    void Enable(ID3D11DeviceContext* context);

private:
    // Conversions from GTEngine values to DX11 values.
    static D3D11_DEPTH_WRITE_MASK const msWriteMask[];
    static D3D11_COMPARISON_FUNC const msComparison[];
    static D3D11_STENCIL_OP const msOperation[];
};

}
