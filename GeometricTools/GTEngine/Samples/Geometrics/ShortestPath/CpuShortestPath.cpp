// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "CpuShortestPath.h"


CpuShortestPath::CpuShortestPath(std::shared_ptr<Texture2> const& weights)
    :
    mSize(static_cast<int>(weights->GetWidth())),
    mWeights(mSize, mSize),
    mNodes(mSize, mSize)
{
    mWeights.SetRawPixels(weights->GetData());
}

void CpuShortestPath::Compute(std::stack<std::pair<int, int>>& path)
{
    // Compute the distances on the top of the grid.  These are simply partial
    // sums of weights along a linear path.
    float distance = 0.0f;
    mNodes(0, 0) = Node(distance, -1, -1);
    for (int x = 1; x < mSize; ++x)
    {
        distance += mWeights(x - 1, 0).w1;
        mNodes(x, 0) = Node(distance, x - 1, 0);
    }

    // Compute the distances on the left edge of the grid.
    distance = 0.0f;
    for (int y = 1; y < mSize; ++y)
    {
        distance += mWeights(0, y - 1).w2;
        mNodes(0, y) = Node(distance, 0, y - 1);
    }

    // The update function for computing the minimum distance at a node using
    // the three incoming edges from its neighbors.
    std::function<void(int, int)> Update =
        [this](int x, int y)
    {
        float dmin = mNodes(x - 1, y).distance + mWeights(x - 1, y).w1;
        mNodes(x, y) = Node(dmin, x - 1, y);
        float d = mNodes(x, y - 1).distance + mWeights(x, y - 1).w2;
        if (d < dmin)
        {
            dmin = d;
            mNodes(x, y) = Node(dmin, x, y - 1);
        }
        d = mNodes(x - 1, y - 1).distance + mWeights(x - 1, y - 1).w3;
        if (d < dmin)
        {
            dmin = d;
            mNodes(x, y) = Node(dmin, x - 1, y - 1);
        }
    };

    // Compute the distances on the segments x+y=z.  NOTE:  The construction
    // uses knowledge that the grid is a square.  The logic is slightly more
    // complicated for a nonsquare grid, because you have to know when the
    // segments transition from an endpoint on the left edge to an endpoint
    // on the bottom edge (width > height) or from an endpoint on the top
    // edge to an endpoint on the right edge (width < height).  In the case
    // of a square, the endpoints are on left-top and transition to
    // bottom-right at the same time.
    for (int z = 2; z < mSize; ++z)
    {
        for (int x = 1, y = z - x; y > 0; ++x, --y)
        {
            Update(x, y);
        }
    }
    for (int z = mSize; z <= 2 * (mSize - 1); ++z)
    {
        for (int y = mSize - 1, x = z - y; x < mSize; --y, ++x)
        {
            Update(x, y);
        }
    }

    // Create the path by starting at (mXSize-1,mYSize-1) and following the
    // previous links.
    int x = mSize - 1, y = mSize - 1;
    while (x != -1)
    {
        path.push(std::make_pair(x, y));
        Node n = mNodes(x, y);
        x = n.xPrevious;
        y = n.yPrevious;
    }
}

CpuShortestPath::Node::Node(float dist, int xPrev, int yPrev)
    :
    distance(dist),
    xPrevious(xPrev),
    yPrevious(yPrev)
{
}

