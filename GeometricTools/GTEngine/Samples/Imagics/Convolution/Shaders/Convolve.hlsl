// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

cbuffer Weights
{
    float weight[2*RADIUS+1][2*RADIUS+1];
};

Texture2D<float4> input;
RWTexture2D<float4> output;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(int2 dt : SV_DispatchThreadID)
{
    float4 result = 0.0f;
    for (int y = -RADIUS; y <= RADIUS; ++y)
    {
        for (int x = -RADIUS; x <= RADIUS; ++x)
        {
            result += weight[y + RADIUS][x + RADIUS] * input[dt + int2(x, y)];
        }
    }
    output[dt] = result;
}
