// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

cbuffer PVWMatrix
{
    float4x4 pvwMatrix;
};

static float4 color[4] =
{
    float4(0.0f, 0.0f, 0.5f, 1.0f),
    float4(0.5f, 0.0f, 0.0f, 1.0f),
    float4(0.0f, 0.5f, 0.5f, 1.0f),
    float4(0.5f, 0.5f, 0.0f, 1.0f)
};

struct VS_INPUT
{
    float3 modelPosition : POSITION;
    uint modelColorIndex : COLOR0;
};

struct VS_OUTPUT
{
    float4 vertexColor : COLOR0;
    float4 clipPosition : SV_POSITION;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;
#if GTE_USE_MAT_VEC
    output.clipPosition = mul(pvwMatrix, float4(input.modelPosition, 1.0f));
#else
    output.clipPosition = mul(float4(input.modelPosition, 1.0f), pvwMatrix);
#endif
    output.vertexColor = color[input.modelColorIndex];
    return output;
}

struct PS_INPUT
{
    float4 vertexColor : COLOR0;
};

struct PS_OUTPUT
{
    float4 pixelColor0 : SV_TARGET0;
};

PS_OUTPUT PSMain(PS_INPUT input)
{
    PS_OUTPUT output;
    output.pixelColor0 = input.vertexColor;
    return output;
};
