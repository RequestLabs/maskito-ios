// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

cbuffer WireParameters
{
    float4 meshColor;
    float4 edgeColor;
    float2 windowSize;
};

//----------------------------------------------------------------------------
cbuffer PVWMatrix
{
    float4x4 pvwMatrix;
};

struct VS_INPUT
{
    float3 modelPosition : POSITION;
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
    output.vertexColor = meshColor;
    return output;
}
//----------------------------------------------------------------------------
struct GS_OUTPUT
{
    float4 vertexColor : COLOR0;
    noperspective float3 edgeDistance : TEXCOORD0;
    float4 clipPosition : SV_POSITION;
};

[maxvertexcount(3)]
void GSMain(triangle VS_OUTPUT input[3], inout TriangleStream<GS_OUTPUT> stream)
{
    GS_OUTPUT output[3];

    float2 pixel[3];
    float W[3];
    int i;
    [unroll]
    for (i = 0; i < 3; ++i)
    {
        float2 ndc = input[i].clipPosition.xy / input[i].clipPosition.w;
        pixel[i] = 0.5f * windowSize * (ndc + 1.0f);
    }

    int j0[3] = { 2, 0, 1 }, j1[3] = { 1, 2, 0 };
    float edgeDistance[3];
    [unroll]
    for (i = 0; i < 3; ++i)
    {
        float2 diff = pixel[i] - pixel[j1[i]];
        float2 edge = pixel[j0[i]] - pixel[j1[i]];
        float edgeLength = length(edge);
        if (edgeLength > 0.0f)
        {
            edgeDistance[i] =
                abs(dot(diff, float2(edge.y, -edge.x)) / edgeLength);
        }
        else
        {
            edgeDistance[i] = 0.0f;
        }

        output[i].vertexColor = input[i].vertexColor;
        output[i].clipPosition = input[i].clipPosition;
    }

    output[0].edgeDistance = float3(edgeDistance[0], 0.0f, 0.0f);
    output[1].edgeDistance = float3(0.0f, edgeDistance[1], 0.0f);
    output[2].edgeDistance = float3(0.0f, 0.0f, edgeDistance[2]);
    stream.Append(output[0]);
    stream.Append(output[1]);
    stream.Append(output[2]);
    stream.RestartStrip();
}
//----------------------------------------------------------------------------
struct PS_OUTPUT
{
    float4 pixelColor0 : SV_TARGET0;
};

PS_OUTPUT PSMain(GS_OUTPUT input)
{
    PS_OUTPUT output;
    float dmin = min(input.edgeDistance[0], input.edgeDistance[1]);
    dmin = min(dmin, input.edgeDistance[2]);
    float blend = smoothstep(0.0f, 1.0f, dmin);
    output.pixelColor0 = lerp(edgeColor, input.vertexColor, blend);
    output.pixelColor0.a = dmin;
    return output;
}
//----------------------------------------------------------------------------
