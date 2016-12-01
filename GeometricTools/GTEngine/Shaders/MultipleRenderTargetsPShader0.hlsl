// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

Texture2D<uint> depthTexture;
Texture2D<float4> positionTexture;
RWTexture2D<float4> colorTexture;
SamplerState nSampler;

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
    float4 pos = positionTexture.Sample(nSampler, input.vertexTCoord);
    uint depthR24S8 = depthTexture[(int2)pos.xy];
    float gray = (depthR24S8 & 0x00FFFFFF) / 16777215.0f;
    output.pixelColor0 = float4(gray, gray, gray, 1.0f);
    colorTexture[(int2)pos.xy] = float4(0.4f, 0.5f, 0.6f, 1.0f);
    return output;
}
