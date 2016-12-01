// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

Texture3D<float> density;
Texture3D<float4> velocity;
RWTexture3D<float4> stateTm1;
RWTexture3D<float4> stateT;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
void CSMain(uint3 c : SV_DispatchThreadID)
{
    float4 initial = float4(velocity[c.xyz].xyz, density[c.xyz]);
    stateTm1[c.xyz] = initial;
    stateT[c.xyz] = initial;
}
