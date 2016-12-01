// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

Texture2D<float4> state;
SamplerState bilinearClampSampler;

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
    // Map velocity channels to colors and modulate by density.
    PS_OUTPUT output;
    float4 current = state.Sample(bilinearClampSampler, input.vertexTCoord);
    float3 color = 0.5f + 0.5f*current.xyz/(1.0f + abs(current.xyz));
    output.pixelColor0 = float4(current.w*color, 1.0f);
    return output;
}
