// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

struct VS_INPUT
{
    float2 modelPosition : POSITION;
    float2 modelTCoord : TEXCOORD0;
};

struct VS_OUTPUT
{
    float2 vertexTCoord : TEXCOORD0;
    float4 clipPosition : SV_POSITION;
};

VS_OUTPUT VSMain (VS_INPUT input)
{
    VS_OUTPUT output;
    output.clipPosition.x = 2.0f*input.modelPosition.x - 1.0f;
    output.clipPosition.y = -2.0f*input.modelPosition.y + 1.0f;
    output.clipPosition.z = 0.0f;
    output.clipPosition.w = 1.0f;
    output.vertexTCoord = input.modelTCoord;
    return output;
}
