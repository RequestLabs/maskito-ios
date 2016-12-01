// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Graphics/DX11/GteHLSLSamplerState.h>
using namespace gte;


HLSLSamplerState::~HLSLSamplerState()
{
}

HLSLSamplerState::HLSLSamplerState(D3D11_SHADER_INPUT_BIND_DESC const& desc)
    :
    HLSLResource(desc, 0)
{
}

HLSLSamplerState::HLSLSamplerState(D3D11_SHADER_INPUT_BIND_DESC const& desc,
    unsigned int index)
    :
    HLSLResource(desc, index, 0)
{
}

