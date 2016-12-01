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
    return output;
}

Texture2DArray myTexture;
SamplerState mySampler;

struct PS_INPUT
{
    float2 vertexTCoord : TEXCOORD0;
};

struct PS_OUTPUT
{
    float4 pixelColor0 : SV_TARGET0;
};

PS_OUTPUT PSMain(PS_INPUT input)
{
    PS_OUTPUT output;
    float3 tcoord0 = float3(input.vertexTCoord, 0);
    float4 color0 = myTexture.Sample(mySampler, tcoord0);
    float3 tcoord1 = float3(input.vertexTCoord, 1);
    float4 color1 = myTexture.Sample(mySampler, tcoord1);
    output.pixelColor0 = 0.5f*(color0 + color1);
    return output;
};
