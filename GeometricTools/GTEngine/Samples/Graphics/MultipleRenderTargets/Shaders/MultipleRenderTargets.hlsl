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

struct VS_INPUT
{
    float3 modelPosition : POSITION;
    float2 modelTCoord : TEXCOORD0;
};

struct VS_OUTPUT
{
    float2 vertexTCoord : TEXCOORD0;
    float perspectiveDepth : TEXCOORD1;
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
    output.vertexTCoord = input.modelTCoord;
    output.perspectiveDepth = output.clipPosition.z / output.clipPosition.w;
    return output;
}

cbuffer FarNearRatio
{
    float farNearRatio;
};

Texture2D<float4> baseTexture;
SamplerState baseSampler;

struct PS_INPUT
{
    float2 vertexTCoord : TEXCOORD0;
    float perspectiveDepth : TEXCOORD1;
    float4 screenPosition : SV_POSITION;
};

struct PS_OUTPUT
{
    float4 color : SV_TARGET0;
    float4 screenPosition : SV_TARGET1;
    float linearDepth : SV_DEPTH;
};

PS_OUTPUT PSMain(PS_INPUT input)
{
    PS_OUTPUT output;
    output.color = baseTexture.Sample(baseSampler, input.vertexTCoord);
    output.screenPosition = input.screenPosition;
    float z = input.perspectiveDepth;
    output.linearDepth = z / (farNearRatio*(1.0f - z) + z);
    return output;
};
