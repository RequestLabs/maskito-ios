// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

Texture2D<float4> input;
RWTexture2D<float4> output;
[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(int2 t : SV_DispatchThreadID)
{
    float4 average = 0.0f;
    for (int y = -DELTA; y <= DELTA; ++y)
    {
        for (int x = -DELTA; x <= DELTA; ++x)
        {
            average += input[t + int2(x, y)];
        }
    }
    average /= (2.0f * DELTA + 1.0f)*(2.0f * DELTA + 1.0f);
    output[t] = average;
}
