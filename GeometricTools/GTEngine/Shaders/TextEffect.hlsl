// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

cbuffer Translate
{
    float2 translate;
};
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
    output.vertexTCoord = input.modelTCoord;
    output.clipPosition.x = 2.0f*input.modelPosition.x - 1.0f + 2.0f*translate.x;
    output.clipPosition.y = 2.0f*input.modelPosition.y - 1.0f + 2.0f*translate.y;
    output.clipPosition.z = 0.0f;
    output.clipPosition.w = 1.0f;
    return output;
}

cbuffer TextColor
{
    float4 textColor;
};

Texture2D baseTexture;
SamplerState baseSampler;

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
    float bitmapAlpha = baseTexture.Sample(baseSampler, input.vertexTCoord).r;
    if (bitmapAlpha > 0.5f)
    {
        discard;
    }
    output.pixelColor0 = textColor;
    return output;
}
