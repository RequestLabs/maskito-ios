// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

cbuffer PVWMatrix { float4x4 pvwMatrix; };
cbuffer FlowDirection { float2 flowDirection; };

struct VS_INPUT
{
    float3 modelPosition : POSITION;
    float2 modelGroundTCoord : TEXCOORD0;
    float modelBlendTCoord : TEXCOORD1;
    float2 modelCloudTCoord : TEXCOORD2;
};

struct VS_OUTPUT
{
    float2 vertexGroundTCoord : TEXCOORD0;
    float vertexBlendTCoord : TEXCOORD1;
    float2 vertexCloudTCoord : TEXCOORD2;
    float2 vertexFlowDirection : TEXCOORD3;
    float4 clipPosition : SV_POSITION;
};

VS_OUTPUT VSMain (VS_INPUT input)
{
    VS_OUTPUT output;

    // Transform the position from model space to clip space.
#if GTE_USE_MAT_VEC
    output.clipPosition = mul(pvwMatrix, float4(input.modelPosition, 1.0f));
#else
    output.clipPosition = mul(float4(input.modelPosition, 1.0f), pvwMatrix);
#endif

    // Pass through the texture coordinates.
    output.vertexGroundTCoord = input.modelGroundTCoord;
    output.vertexBlendTCoord = input.modelBlendTCoord;
    output.vertexCloudTCoord = input.modelCloudTCoord;

    // Pass through the flow direction, to be used as an offset in the pixel
    // program.
    output.vertexFlowDirection = flowDirection;
    return output;
}

Texture2D<float4> grassTexture;
Texture2D<float4> stoneTexture;
Texture1D<float4> blendTexture;
Texture2D<float4> cloudTexture;
SamplerState commonSampler;  // minLmagLmipL, repeat
SamplerState blendSampler;  // minPmagLmipP, repeat

cbuffer PowerFactor { float powerFactor; };

struct PS_INPUT
{
    float2 vertexGroundTCoord : TEXCOORD0;
    float vertexBlendTCoord : TEXCOORD1;
    float2 vertexCloudTCoord : TEXCOORD2;
    float2 vertexFlowDirection : TEXCOORD3;
};

float4 PSMain (PS_INPUT input) : SV_TARGET
{
    float4 grassColor = grassTexture.Sample(commonSampler, input.vertexGroundTCoord);
    float4 stoneColor = stoneTexture.Sample(commonSampler, input.vertexGroundTCoord);
    float4 blendColor = blendTexture.Sample(blendSampler, input.vertexBlendTCoord);

    float2 offsetCloudTCoord = input.vertexCloudTCoord + input.vertexFlowDirection;    
    float4 cloudColor = cloudTexture.Sample(commonSampler, offsetCloudTCoord);

    float stoneWeight = pow(abs(blendColor.r), powerFactor);
    float grassWeight = 1.0f - stoneWeight;
    float4 pixelColor = cloudColor*(grassWeight*grassColor + stoneWeight*stoneColor);
    return pixelColor;
}
