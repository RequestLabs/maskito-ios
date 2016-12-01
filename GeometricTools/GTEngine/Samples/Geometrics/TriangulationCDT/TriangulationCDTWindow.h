// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
using namespace gte;

class TriangulationCDTWindow : public Window
{
public:
    TriangulationCDTWindow(Parameters& parameters);

    virtual void OnDisplay();
    virtual bool OnCharPress(unsigned char key, int x, int y);

private:
    void DrawLine(int x0, int y0, int x1, int y1);
    void DrawTriangulation();

    void UnindexedSimplePolygon();  // key = '0'
    void IndexedSimplePolygon();    // key = '1'
    void OneNestedPolygon();        // key = '2'
    void TwoNestedPolygons();       // key = '3'
    void TreeOfNestedPolygons();    // key = '4'

    std::shared_ptr<OverlayEffect> mOverlay;
    std::shared_ptr<Texture2> mScreen;
    unsigned int* mScreenTexels;

    typedef BSNumber<UIntegerAP32> Rational;
    typedef TriangulateCDT<float, Rational> Triangulator;
    typedef PlanarMesh<float, Rational, Rational> PlanarMesher;
    std::vector<Vector2<float>> mPoints;
    std::unique_ptr<Triangulator> mTriangulator;
    std::unique_ptr<PlanarMesher> mPMesher;
};
