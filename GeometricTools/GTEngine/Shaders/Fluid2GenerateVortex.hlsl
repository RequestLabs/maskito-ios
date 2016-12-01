// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

cbuffer Parameters
{
    float4 spaceDelta;    // (dx, dy, 0, 0)
    float4 halfDivDelta;  // (0.5/dx, 0.5/dy, 0, 0)
    float4 timeDelta;     // (dt/dx, dt/dy, 0, dt)
    float4 viscosityX;    // (velVX, velVX, 0, denVX)
    float4 viscosityY;    // (velVX, velVY, 0, denVY)
    float4 epsilon;       // (epsilonX, epsilonY, 0, epsilon0)
};

cbuffer Vortex
{
    float4 data;  // (x, y, variance, amplitude)
};

Texture2D<float2> inVelocity;
RWTexture2D<float2> outVelocity;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(uint3 c : SV_DispatchThreadID)
{
    float2 location = spaceDelta.xy*(c.xy + 0.5f);
    float2 diff = location - data.xy;
    float arg = -dot(diff, diff) / data.z;
    float magnitude = data.w*exp(arg);
    float2 vortexVelocity = magnitude*float2(diff.y, -diff.x);
    outVelocity[c.xy] = inVelocity[c.xy] + vortexVelocity;
}
