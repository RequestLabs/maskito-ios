// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
#include <stack>
using namespace gte;

class CpuShortestPath
{
public:
    CpuShortestPath(std::shared_ptr<Texture2> const& weights);
    void Compute(std::stack<std::pair<int, int>>& path);

private:
    // The weights texture stores (F(x,y), W1(x,y), W2(x,y), W3(x,y)), where
    // F(x,y) is the height field and the edge weights are
    //   W1(x,y) = W((x,y),(x+1,y))   = (F(x+1,y) + F(x,y))/2
    //   W2(x,y) = W((x,y),(x,y+1))   = (F(x,y+1) + F(x,y))/2
    //   W3(x,y) = W((x,y),(x+1,y+1)) = (F(x+1,y+1) + F(x,y))/sqrt(2)
    struct Weights
    {
        // For more readable code using names rather than indices for
        // components of Vector4<float>
        float h, w1, w2, w3;
    };

    // The minimum distance to pixel at (x,y) and the previous neighbor
    // (xPrevious,yPrevious) that led to this minimum.
    struct Node
    {
        Node(float dist = 0.0f, int xPrev = -1, int yPrev = -1);
        float distance;
        int xPrevious, yPrevious;
    };

    // The 'weights' input is mSize-by-mSize.
    int mSize;

    // Use the Image2 object to access 'weights' using 2-tuple locations.
    Image2<Weights> mWeights;

    // Keep track of the distances and the previous pixel that led to the
    // minimum distance for the current pixel.
    Image2<Node> mNodes;
};
