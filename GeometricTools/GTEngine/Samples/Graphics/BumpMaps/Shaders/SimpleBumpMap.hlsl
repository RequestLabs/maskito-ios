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
    float3 modelLightDirection : COLOR;
    float2 modelBaseTCoord : TEXCOORD0;
    float2 modelNormalTCoord : TEXCOORD1;
};

struct VS_OUTPUT
{
    float3 vertexLightDirection : COLOR;
    float2 vertexBaseTCoord : TEXCOORD0;
    float2 vertexNormalTCoord : TEXCORD1;
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

    output.vertexLightDirection = input.modelLightDirection;
    output.vertexBaseTCoord = input.modelBaseTCoord;
    output.vertexNormalTCoord = input.modelNormalTCoord;
    return output;
}

Texture2D<float4> baseTexture;
Texture2D<float4> normalTexture;
SamplerState commonSampler;

struct PS_INPUT
{
    float3 vertexLightDirection : COLOR;
    float2 vertexBaseTCoord : TEXCOORD0;
    float2 vertexNormalTCoord : TEXCOORD1;
};

struct PS_OUTPUT
{
    float4 pixelColor : SV_TARGET;
};

PS_OUTPUT PSMain(PS_INPUT input)
{
    PS_OUTPUT output;
    float3 baseColor = baseTexture.Sample(commonSampler, input.vertexBaseTCoord).rgb;
    float3 normalColor = normalTexture.Sample(commonSampler, input.vertexNormalTCoord).rgb;
    float3 lightDirection = 2.0f * input.vertexLightDirection - 1.0f;
    float3 normalDirection = 2.0f * normalColor - 1.0f;
    float LdN = saturate(dot(lightDirection, normalDirection));
    output.pixelColor = float4(LdN * baseColor, 1.0f);
    return output;
}
