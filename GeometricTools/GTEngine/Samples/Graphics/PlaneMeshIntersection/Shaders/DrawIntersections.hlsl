// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

Texture2D<float4> color;
Texture2D<float2> planeConstant;
RWTexture2D<float4> output;

[numthreads(8,8,1)]
void CSMain(int2 t : SV_DISPATCHTHREADID)
{
    uint value = asuint(color[t].w);
    if (value == 0x7F7FFFFF)
    {
        output[t] = float4(color[t].rgb, 1.0f);
        return;
    }

    float signChange = 0.0f;
    float2 intValue = floor(planeConstant[t]);
    float2 diff0 = planeConstant[t] - intValue;
    float2 diff1 = 0.0f;
    if (diff0.x > 0.0f || diff0.y > 0.0f)
    {
        int2 nbr;
        for (int dy = -1; dy <= 1; ++dy)
        {
            nbr.y = t.y + dy;
            for (int dx = -1; dx <= 1; ++dx)
            {
                nbr.x = t.x + dx;
                diff1 = intValue - planeConstant[nbr];
                if (diff1.x >= 0.0f || diff1.y >= 0.0f)
                {
                    signChange = 1.0f;
                }
            }
        }
    }
    else
    {
        signChange = 1.0f;
    }

    output[t] = signChange*float4(0.0f, 0.0f, 0.0f, 1.0f) +
        (1.0f - signChange)*float4(color[t].rgb, 1.0f);
}
