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

Texture1DArray myTexture1;  // 2 textures in the array
Texture2DArray myTexture2;  // 2 textures in the array
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
    output.pixelColor0 = 0.0f;

    float4 tcd;

    // Sample the 1D texture array.
    tcd.xy = float2(input.vertexTCoord.x, 0);
    output.pixelColor0 += myTexture1.Sample(mySampler, tcd.xy);
    tcd.xy = float2(input.vertexTCoord.x, 1);
    output.pixelColor0 += myTexture1.Sample(mySampler, tcd.xy);

    // Sample the 2D texture array.
    tcd.xyz = float3(input.vertexTCoord, 0);
    output.pixelColor0 += myTexture2.Sample(mySampler, tcd.xyz);
    tcd.xyz = float3(input.vertexTCoord, 1);
    output.pixelColor0 += myTexture2.Sample(mySampler, tcd.xyz);

    output.pixelColor0 *= 0.25f;
    return output;
};
