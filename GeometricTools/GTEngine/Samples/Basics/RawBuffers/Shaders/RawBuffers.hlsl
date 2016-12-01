// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

// 16 bytes ('a', pi<double>, pi<float>, -1, 'b')
ByteAddressBuffer input;

// 16 bytes (pi<double>, pi<float>, -1, 'a', 'b')
RWByteAddressBuffer output;

[numthreads(1, 1, 1)]
void CSMain(int3 t : SV_DispatchThreadID)
{
    uint4 inValue = input.Load4(0);

    // Extract character 'a'.
    uint a = inValue.x & 0x000000FF;

    // Extract double-precision pi.
    uint pidLoEncoding = (inValue.x >> 8) | ((inValue.y & 0x000000FF) << 24);
    uint pidHiEncoding = (inValue.y >> 8) | ((inValue.z & 0x000000FF) << 24);
    double pid = asdouble(pidLoEncoding, pidHiEncoding);

    // Extract single-precision pi.
    uint pifEncoding = (inValue.z >> 8) | ((inValue.w & 0x000000FF) << 24);
    float pif = asfloat(pifEncoding);

    // Extract short -1.
    uint minusOneEncoding = (inValue.w >> 8) & 0x0000FFFF;
    int minusOne = asint(minusOneEncoding) >> 16;

    // Extract character 'b'.
    uint b = (inValue.w >> 24);

    // Return the repackaged input.  Although we already know the uint values
    // are the same as extracted, this code shows how to reinterpret 'float'
    // and 'double' values.
    asuint(pid, pidLoEncoding, pidHiEncoding);
    pifEncoding = asuint(pif);
    uint4 outValue;
    outValue.x = pidLoEncoding;
    outValue.y = pidHiEncoding;
    outValue.z = pifEncoding;
    outValue.w = minusOneEncoding | (a << 16) | (b << 24);

    output.Store4(0, outValue);
}
