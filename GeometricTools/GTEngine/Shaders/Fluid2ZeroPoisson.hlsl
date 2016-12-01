// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

RWTexture2D<float> poisson;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(uint2 c : SV_DispatchThreadID)
{
    poisson[c] = 0.0f;
}
