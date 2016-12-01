// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

struct VS_STRUCT
{
    float3 position : POSITION;
    float3 color : COLOR0;
    float size : TEXCOORD0;
};

VS_STRUCT VSMain (VS_STRUCT input)
{
    return input;
}

struct GS_OUTPUT
{
    float3 color : COLOR0;
    float4 clipPosition : SV_POSITION;
};

cbuffer Matrices
{
    float4x4 vwMatrix;
    float4x4 pMatrix;
};

static float4 offset[4] =
{
    float4(-1.0f, -1.0f, 0.0f, 0.0f),
    float4(+1.0f, -1.0f, 0.0f, 0.0f),
    float4(-1.0f, +1.0f, 0.0f, 0.0f),
    float4(+1.0f, +1.0f, 0.0f, 0.0f)
};

[maxvertexcount(6)]
void GSMain (point VS_STRUCT input[1], inout TriangleStream<GS_OUTPUT> stream)
{
    GS_OUTPUT output[4];
    float4 viewPosition = mul(vwMatrix, float4(input[0].position, 1.0f));
    int i;
    for (i = 0; i < 4; ++i)
    {
        float4 corner = viewPosition + input[0].size*offset[i];
        output[i].clipPosition = mul(pMatrix, corner);
        output[i].color = input[0].color;
    }

    stream.Append(output[0]);
    stream.Append(output[1]);
    stream.Append(output[3]);
    stream.RestartStrip();

    stream.Append(output[0]);
    stream.Append(output[3]);
    stream.Append(output[2]);
    stream.RestartStrip();
}

struct PS_OUTPUT
{
    float4 pixelColor0 : SV_TARGET0;
};

PS_OUTPUT PSMain(GS_OUTPUT input)
{
    PS_OUTPUT output;
    output.pixelColor0 = float4(input.color, 1.0f);
    return output;
}
