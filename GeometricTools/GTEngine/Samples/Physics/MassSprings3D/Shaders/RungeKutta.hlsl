// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

cbuffer SimulationParameters
{
    int4 dimensions;    // (columns, rows, slices, columns*rows)
    float viscosity;
    float time;
    float delta;
    float halfDelta;    // delta/2
    float sixthDelta;   // delta/6
    float halfTime;     // time + halfDelta
    float fullTime;     // time + delta
};

StructuredBuffer<float> invMass;
StructuredBuffer<float> constantC;
StructuredBuffer<float> lengthC;
StructuredBuffer<float> constantR;
StructuredBuffer<float> lengthR;
StructuredBuffer<float> constantS;
StructuredBuffer<float> lengthS;

struct Temporary
{
    float3 d1, d2, d3, d4;
};

RWStructuredBuffer<float3> pTmp;
RWStructuredBuffer<Temporary> pAllTmp;  // packed dpTmp1, dpTmp2, dpTmp3, dpTmp4
RWStructuredBuffer<float3> vTmp;
RWStructuredBuffer<Temporary> vAllTmp;  // packed dvTmp1, dvTmp2, dvTmp3, dvTmp4
RWStructuredBuffer<float3> position;
RWStructuredBuffer<float3> velocity;

float3 Acceleration(int i, int3 dt, int4 dimensions,
    float viscosity, StructuredBuffer<float> invMass,
    RWStructuredBuffer<float3> position, RWStructuredBuffer<float3> velocity,
    StructuredBuffer<float> constantC, StructuredBuffer<float> lengthC,
    StructuredBuffer<float> constantR, StructuredBuffer<float> lengthR,
    StructuredBuffer<float> constantS, StructuredBuffer<float> lengthS)
{
    float3 diff, force;
    float ratio;
    int prev, next;

    // Initialize with the external acceleration.
    float3 acc = -viscosity*velocity[i];

    if (dt.x > 0)
    {
        prev = i - 1;  // index to previous column
        diff = position[prev] - position[i];
        ratio = lengthC[prev] / length(diff);
        force = constantC[prev] * (1.0f - ratio) * diff;
        acc += invMass[i] * force;
    }

    if (dt.x < dimensions.x - 1)
    {
        next = i + 1;  // index to next column
        diff = position[next] - position[i];
        ratio = lengthC[i] / length(diff);
        force = constantC[i] * (1.0f - ratio) * diff;
        acc += invMass[i] * force;
    }

    if (dt.y > 0)
    {
        prev = i - dimensions.x;  // index to previous row
        diff = position[prev] - position[i];
        ratio = lengthR[prev] / length(diff);
        force = constantR[prev] * (1.0f - ratio) * diff;
        acc += invMass[i] * force;
    }

    if (dt.y < dimensions.y - 1)
    {
        next = i + dimensions.x;  // index to next row
        diff = position[next] - position[i];
        ratio = lengthR[i] / length(diff);
        force = constantR[i] * (1.0f - ratio) * diff;
        acc += invMass[i] * force;
    }

    if (dt.z > 0)
    {
        prev = i - dimensions.w;  // index to previous slice
        diff = position[prev] - position[i];
        ratio = lengthS[prev] / length(diff);
        force = constantS[prev] * (1.0f - ratio) * diff;
        acc += invMass[i] * force;
    }

    if (dt.z < dimensions.z - 1)
    {
        next = i + dimensions.w;  // index to next slice
        diff = position[next] - position[i];
        ratio = lengthS[i] / length(diff);
        force = constantS[i] * (1.0f - ratio) * diff;
        acc += invMass[i] * force;
    }

    return acc;
}

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
void RK4Step1a(int3 dt : SV_DispatchThreadID)
{
    int i = dt.x + dimensions.x * (dt.y + dimensions.y * dt.z);
    if (invMass[i] > 0.0f)
    {
        pAllTmp[i].d1 = velocity[i];
        vAllTmp[i].d1 = Acceleration(i, dt, dimensions, viscosity, invMass,
            position, velocity, constantC, lengthC, constantR, lengthR,
            constantS, lengthS);
    }
}

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
void RK4Step1b(int3 dt : SV_DispatchThreadID)
{
    int i = dt.x + dimensions.x * (dt.y + dimensions.y * dt.z);
    if (invMass[i] > 0.0f)
    {
        pTmp[i] = position[i] + halfDelta*pAllTmp[i].d1;
        vTmp[i] = velocity[i] + halfDelta*vAllTmp[i].d1;
    }
    else
    {
        pTmp[i] = position[i];
        vTmp[i] = 0.0f;
    }
}

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
void RK4Step2a(int3 dt : SV_DispatchThreadID)
{
    int i = dt.x + dimensions.x * (dt.y + dimensions.y * dt.z);
    if (invMass[i] > 0.0f)
    {
        pAllTmp[i].d2 = velocity[i];
        vAllTmp[i].d2 = Acceleration(i, dt, dimensions, viscosity, invMass,
            pTmp, vTmp, constantC, lengthC, constantR, lengthR,
            constantS, lengthS);
    }
}

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
void RK4Step2b(int3 dt : SV_DispatchThreadID)
{
    int i = dt.x + dimensions.x * (dt.y + dimensions.y * dt.z);
    if (invMass[i] > 0.0f)
    {
        pTmp[i] = position[i] + halfDelta*pAllTmp[i].d2;
        vTmp[i] = velocity[i] + halfDelta*vAllTmp[i].d2;
    }
    else
    {
        pTmp[i] = position[i];
        vTmp[i] = 0.0f;
    }
}

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
void RK4Step3a(int3 dt : SV_DispatchThreadID)
{
    int i = dt.x + dimensions.x * (dt.y + dimensions.y * dt.z);
    if (invMass[i] > 0.0f)
    {
        pAllTmp[i].d3 = velocity[i];
        vAllTmp[i].d3 = Acceleration(i, dt, dimensions, viscosity, invMass,
            pTmp, vTmp, constantC, lengthC, constantR, lengthR,
            constantS, lengthS);
    }
}

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
void RK4Step3b(int3 dt : SV_DispatchThreadID)
{
    int i = dt.x + dimensions.x * (dt.y + dimensions.y * dt.z);
    if (invMass[i] > 0.0f)
    {
        pTmp[i] = position[i] + delta*pAllTmp[i].d3;
        vTmp[i] = velocity[i] + delta*vAllTmp[i].d3;
    }
    else
    {
        pTmp[i] = position[i];
        vTmp[i] = 0.0f;
    }
}

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
void RK4Step4a(int3 dt : SV_DispatchThreadID)
{
    int i = dt.x + dimensions.x * (dt.y + dimensions.y * dt.z);
    if (invMass[i] > 0.0f)
    {
        pAllTmp[i].d4 = velocity[i];
        vAllTmp[i].d4 = Acceleration(i, dt, dimensions, viscosity, invMass,
            pTmp, vTmp, constantC, lengthC, constantR, lengthR,
            constantS, lengthS);
    }
}

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
void RK4Step4b(int3 dt : SV_DispatchThreadID)
{
    int i = dt.x + dimensions.x * (dt.y + dimensions.y * dt.z);
    if (invMass[i] > 0.0f)
    {
        position[i] += sixthDelta*(
            pAllTmp[i].d1 + 2.0f*(pAllTmp[i].d2 + pAllTmp[i].d3) + pAllTmp[i].d4);
        velocity[i] += sixthDelta*(
            vAllTmp[i].d1 + 2.0f*(vAllTmp[i].d2 + vAllTmp[i].d3) + vAllTmp[i].d4);
    }
}
