// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

Buffer<float4> input;
RWBuffer<float4> output;

[numthreads(1, 1, 1)]
void CSMain(int3 t : SV_DispatchThreadID)
{
    output[0] = input[0].abgr;
}
