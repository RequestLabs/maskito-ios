// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

cbuffer Weights
{
    float weight[2*RADIUS+1];
};

Texture2D<float4> input;
RWTexture2D<float4> output;

// See the comments in ConvolveSeparableHGS2.hlsl.

#define NUM_Y_THREADS 192
groupshared float4 samples[NUM_Y_THREADS + 2*RADIUS];

// numXGroups = 1024, numYGroups = 4
[numthreads(1, NUM_Y_THREADS, 1)]
void CSMain(int2 dt : SV_DispatchThreadID, int2 gt : SV_GroupThreadID)
{
    // Load the texels from the input texture, store them in group-shared
    // memory, and have all threads in the group wait until all texels
    // are loaded.
    samples[gt.y + RADIUS] = input[dt];
    if (gt.y < RADIUS)
    {
        samples[gt.y] = input[dt - int2(0, RADIUS)];
    }
    else if (gt.y >= NUM_Y_THREADS - RADIUS)
    {
        samples[gt.y + 2 * RADIUS] = input[dt + int2(0, RADIUS)];
    }
    GroupMemoryBarrierWithGroupSync();

    float4 result = 0.0f;
    for (int y = 0; y <= 2*RADIUS; ++y)
    {
        result += weight[y] * samples[gt.y + y];
    }
    output[dt] = result;
}
