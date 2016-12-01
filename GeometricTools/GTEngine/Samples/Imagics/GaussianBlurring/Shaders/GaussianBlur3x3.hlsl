// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

Texture2D<float4> input;
RWTexture2D<float4> output;

static float weight[3][3] =
{
    { 1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f },
    { 2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f },
    { 1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f }
};

static int2 offset[3][3] =
{
    { int2(-1, -1), int2(0, -1), int2(+1, -1) },
    { int2(-1,  0), int2(0,  0), int2(+1,  0) },
    { int2(-1, +1), int2(0, +1), int2(+1, +1) }
};

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(int2 t : SV_DispatchThreadID)
{
    float4 result = 0.0f;
    [unroll]
    for (int r = 0; r < 3; ++r)
    {
        [unroll]
        for (int c = 0; c < 3; ++c)
        {
            result += weight[r][c] * input[t + offset[r][c]];
        }
    }
    output[t] = float4(result.rgb, 1.0f);
}
