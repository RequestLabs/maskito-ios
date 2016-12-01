// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

//----------------------------------------------------------------------------
struct VS_INPUT
{
    uint id : SV_VertexID;
};

struct VS_OUTPUT
{
    uint id : TEXCOORD0;
};

VS_OUTPUT VSMain (VS_INPUT input)
{
    VS_OUTPUT output;
    output.id = input.id;
    return output;
}
//----------------------------------------------------------------------------
struct Particle
{
    float3 position;
    float3 color;
    float size;
};

StructuredBuffer<Particle> particles;

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
void GSMain (point VS_OUTPUT input[1], inout TriangleStream<GS_OUTPUT> stream)
{
    Particle particle = particles[input[0].id];

    GS_OUTPUT output[4];
#if GTE_USE_MAT_VEC
    float4 viewPosition = mul(vwMatrix, float4(particle.position, 1.0f));
#else
    float4 viewPosition = mul(float4(particle.position, 1.0f), vwMatrix);
#endif
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        float4 corner = viewPosition + particle.size*offset[i];
#if GTE_USE_MAT_VEC
        output[i].clipPosition = mul(pMatrix, corner);
#else
        output[i].clipPosition = mul(corner, pMatrix);
#endif
        output[i].color = particle.color;
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
//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
