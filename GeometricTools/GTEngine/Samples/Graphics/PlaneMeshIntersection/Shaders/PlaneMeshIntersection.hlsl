// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

cbuffer PMIParameters
{
    float4x4 pvMatrix;
    float4x4 wMatrix;

    // The planes are of form Dot(planeVector,(x,y,z,1)) = i for integer i
    // where (x,y,z,1) is the world position.
    float4 planeVector0;
    float4 planeVector1;
};

struct VS_INPUT
{
    float3 modelPosition : POSITION;
};

struct VS_OUTPUT
{
    float3 vertexColor : COLOR0;
    noperspective float2 planeConstant : TEXCOORD0;
    float4 clipPosition : SV_POSITION;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;

    output.vertexColor = float3(0.0f, 0.0f, 1.0f);

    // Compute the world position of the vertex.
    float4 modelPosition = float4(input.modelPosition, 1.0f);
    float4 worldPosition;
#if GTE_USE_MAT_VEC
    worldPosition = mul(wMatrix, modelPosition);
#else
    worldPosition = mul(modelPosition, wMatrix);
#endif
    // Compute the plane constant c of the vertex.
    output.planeConstant.x = dot(planeVector0, worldPosition);
    output.planeConstant.y = dot(planeVector1, worldPosition);

    // Compute the clip position of the vertex.
#if GTE_USE_MAT_VEC
    output.clipPosition = mul(pvMatrix, worldPosition);
#else
    output.clipPosition = mul(worldPosition, pvMatrix);
#endif

    return output;
}

struct PS_INPUT
{
    float3 vertexColor : COLOR0;
    noperspective float2 planeConstant : TEXCOORD0;
};

struct PS_OUTPUT
{
    float4 pixelColor : SV_TARGET0;
    float2 planeConstant : SV_TARGET1;
};

PS_OUTPUT PSMain(PS_INPUT input)
{
    PS_OUTPUT output;
    output.pixelColor = float4(input.vertexColor, 1.0f);
    output.planeConstant = input.planeConstant;
    return output;
}
