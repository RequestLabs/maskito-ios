// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>

#if defined(WIN32)

#include <Mathematics/GteGenerateMeshUV.h>
using namespace gte;

// TODO: Write this shader.
std::string const GenerateMeshUVBase::msGLSLSource = "";

std::string const GenerateMeshUVBase::msHLSLSource =
"cbuffer Bounds\n"
"{\n"
"    int2 bound;\n"
"    int numBoundaryEdges;\n"
"    int numInputs;\n"
"};\n"
"\n"
"struct VertexGraphData\n"
"{\n"
"    int adjacent;\n"
"    Real weight;\n"
"};\n"
"\n"
"StructuredBuffer<int3> vertexGraph;\n"
"StructuredBuffer<VertexGraphData> vertexGraphData;\n"
"StructuredBuffer<int> orderedVertices;\n"
"StructuredBuffer<Real2> inTCoords;\n"
"RWStructuredBuffer<Real2> outTCoords;\n"
"\n"
"[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]\n"
"void CSMain(int2 t : SV_DispatchThreadID)\n"
"{\n"
"    int index = t.x + bound.x * t.y;\n"
"    if (step(index, numInputs-1))\n"
"    {\n"
"        int v = orderedVertices[numBoundaryEdges + index];\n"
"        int2 range = vertexGraph[v].yz;\n"
"        Real2 tcoord = Real2(0, 0);\n"
"        Real weightSum = 0;\n"
"        for (int j = 0; j < range.y; ++j)\n"
"        {\n"
"            VertexGraphData data = vertexGraphData[range.x + j];\n"
"            weightSum += data.weight;\n"
"            tcoord += data.weight * inTCoords[data.adjacent];\n"
"        }\n"
"        tcoord /= weightSum;\n"
"        outTCoords[v] = tcoord;\n"
"    }\n"
"}\n";

std::string const* GenerateMeshUVBase::msSource[] =
{
    &msGLSLSource,
    &msHLSLSource
};

#endif
