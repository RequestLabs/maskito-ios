// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
using namespace gte;

class TriangulationECWindow : public Window
{
public:
    TriangulationECWindow(Parameters& parameters);

    virtual void OnDisplay();
    virtual bool OnCharPress(unsigned char key, int x, int y);

private:
    void DrawLine(int x0, int y0, int x1, int y1);
    void DrawFloodFill(int x, int y);
    void ClearAll();
    void UnindexedSimplePolygon();  // mExample = 0
    void IndexedSimplePolygon();    // mExample = 1
    void OneNestedPolygon();        // mExample = 2
    void TwoNestedPolygons();       // mExample = 3
    void TreeOfNestedPolygons();    // mExample = 4

    // The inputs (i0,i1,i2) are a permutation of (0,1,2).  The goal is to
    // trap algorithm errors due to order of inner polygons.
    // mExample = 5, 6, 7, 8, 9, 10
    void FourBoxesThreeNested(int i0, int i1, int i2);

    typedef BSRational<UIntegerAP32> Rational;
    typedef TriangulateEC<float, Rational> Triangulator;

    std::vector<Vector2<float>> mPositions;
    std::vector<int> mOuter, mInner0, mInner1, mInner2;
    Triangulator::Tree* mTree;
    std::vector<Vector2<float>> mFillSeeds;
    std::vector<std::array<int, 3>> mTriangles;
    int mExample;

    unsigned int mColor, mBlack, mBlue, mWhite;
    std::shared_ptr<OverlayEffect> mOverlay;
    std::shared_ptr<Texture2> mScreenTexture;
    unsigned int* mScreenTexels;
};
