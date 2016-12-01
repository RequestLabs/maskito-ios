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

cbuffer ConstantColor
{
    float4 constantColor;
};

StructuredBuffer<float3> position;

struct VS_INPUT
{
    uint id : SV_VertexID;
};

struct VS_OUTPUT
{
    float4 vertexColor : COLOR0;
    float4 clipPosition : SV_POSITION;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;
    float3 modelPosition = position[input.id];
#if GTE_USE_MAT_VEC
    output.clipPosition = mul(pvwMatrix, float4(modelPosition, 1.0f));
#else
    output.clipPosition = mul(float4(modelPosition, 1.0f), pvwMatrix);
#endif
    output.vertexColor = constantColor;
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
