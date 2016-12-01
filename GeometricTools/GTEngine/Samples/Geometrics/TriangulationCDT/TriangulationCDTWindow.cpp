// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "TriangulationCDTWindow.h"
#include <iostream>


TriangulationCDTWindow::TriangulationCDTWindow(Parameters& parameters)
    :
    Window(parameters)
{
    mScreen = std::make_shared<Texture2>(DF_R8G8B8A8_UNORM, mXSize, mYSize);
    mScreen->SetUsage(Resource::DYNAMIC_UPDATE);
    memset(mScreen->GetData(), 0xFF, mScreen->GetNumBytes());
    mScreenTexels = mScreen->Get<unsigned int>();

    mOverlay = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, mXSize, mYSize, SamplerState::MIN_P_MAG_P_MIP_P,
        SamplerState::CLAMP, SamplerState::CLAMP, true);
    mOverlay->SetTexture(mScreen);

    UnindexedSimplePolygon();
}

void TriangulationCDTWindow::OnDisplay()
{
    mEngine->Update(mScreen);
    mEngine->Draw(mOverlay);
    mEngine->DisplayColorBuffer(0);
}

bool TriangulationCDTWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case '0':
        UnindexedSimplePolygon();
        OnDisplay();
        return true;
    case '1':
        IndexedSimplePolygon();
        OnDisplay();
        return true;
    case '2':
        OneNestedPolygon();
        OnDisplay();
        return true;
    case '3':
        TwoNestedPolygons();
        OnDisplay();
        return true;
    case '4':
        TreeOfNestedPolygons();
        OnDisplay();
        return true;
    }

    return Window::OnCharPress(key, x, y);
}

void TriangulationCDTWindow::DrawLine(int x0, int y0, int x1, int y1)
{
    ImageUtility2::DrawLine(x0, y0, x1, y1,
        [this](int x, int y)
        {
            mScreenTexels[x + mXSize * (mYSize - 1 - y)] = 0xFF000000;
        }
    );
}

void TriangulationCDTWindow::DrawTriangulation()
{
    memset(mScreen->GetData(), 0xFF, mScreen->GetNumBytes());

    Vector2<float> pmin, pmax;
    ComputeExtremes((int)mPoints.size(), &mPoints[0], pmin, pmax);
    int xmin = (int)floor(pmin[0]);
    int ymin = (int)floor(pmin[1]);
    int xmax = (int)ceil(pmax[0]);
    int ymax = (int)ceil(pmax[1]);

    int tStart = 0;
    for (int y = ymin; y <= ymax; ++y)
    {
        std::cout << "y = " << y << std::endl;
        for (int x = xmin; x <= xmax; ++x)
        {
            Vector2<float> test{ (float)x, (float)y };
            int t = mPMesher->GetContainingTriangle(test, tStart);
            if (t >= 0)
            {
                if (mTriangulator->IsInside(t))
                {
                    mScreenTexels[x + mXSize * (mYSize - 1 - y)] = 0xFFFF8000;
                }
                else
                {
                    mScreenTexels[x + mXSize * (mYSize - 1 - y)] = 0xFF0080FF;
                }

                tStart = t;
            }
            else
            {
                tStart = 0;
            }
        }
    }

    for (auto const& tri : mTriangulator->GetAllTriangles())
    {
        int v0 = tri[0];
        int v1 = tri[1];
        int v2 = tri[2];

        int x0 = (int)mPoints[v0][0];
        int y0 = (int)mPoints[v0][1];
        int x1 = (int)mPoints[v1][0];
        int y1 = (int)mPoints[v1][1];
        DrawLine(x0, y0, x1, y1);

        x0 = (int)mPoints[v1][0];
        y0 = (int)mPoints[v1][1];
        x1 = (int)mPoints[v2][0];
        y1 = (int)mPoints[v2][1];
        DrawLine(x0, y0, x1, y1);

        x0 = (int)mPoints[v2][0];
        y0 = (int)mPoints[v2][1];
        x1 = (int)mPoints[v0][0];
        y1 = (int)mPoints[v0][1];
        DrawLine(x0, y0, x1, y1);
    }
}

void TriangulationCDTWindow::UnindexedSimplePolygon()
{
    int numPoints = 10;
    mPoints.resize(numPoints);
    mPoints[0] = { 58.0f, 278.0f };
    mPoints[1] = { 156.0f, 198.0f };
    mPoints[2] = { 250.0f, 282.0f };
    mPoints[3] = { 328.0f, 232.0f };
    mPoints[4] = { 402.0f, 336.0f };
    mPoints[5] = { 314.0f, 326.0f };
    mPoints[6] = { 274.0f, 400.0f };
    mPoints[7] = { 196.0f, 268.0f };
    mPoints[8] = { 104.0f, 292.0f };
    mPoints[9] = { 110.0f, 382.0f };

    mTriangulator = std::make_unique<Triangulator>(numPoints, &mPoints[0]);
    (*mTriangulator)();
    auto const& triangles = mTriangulator->GetAllTriangles();
    int numTriangles = (int)triangles.size();
    mPMesher = std::make_unique<PlanarMesher>(numPoints, &mPoints[0],
        numTriangles, (int const*)&triangles[0]);

    DrawTriangulation();
}

void TriangulationCDTWindow::IndexedSimplePolygon()
{
    int numPoints = 20;
    mPoints.resize(numPoints);
    mPoints[0] = { 58.0f, 278.0f };
    mPoints[1] = { 0.0f, 0.0f };
    mPoints[2] = { 156.0f, 198.0f };
    mPoints[3] = { 0.0f, 0.0f };
    mPoints[4] = { 250.0f, 282.0f };
    mPoints[5] = { 0.0f, 0.0f };
    mPoints[6] = { 328.0f, 232.0f };
    mPoints[7] = { 0.0f, 0.0f };
    mPoints[8] = { 402.0f, 336.0f };
    mPoints[9] = { 0.0f, 0.0f };
    mPoints[10] = { 314.0f, 326.0f };
    mPoints[11] = { 0.0f, 0.0f };
    mPoints[12] = { 274.0f, 400.0f };
    mPoints[13] = { 0.0f, 0.0f };
    mPoints[14] = { 196.0f, 268.0f };
    mPoints[15] = { 0.0f, 0.0f };
    mPoints[16] = { 104.0f, 292.0f };
    mPoints[17] = { 0.0f, 0.0f };
    mPoints[18] = { 110.0f, 382.0f };
    mPoints[19] = { 0.0f, 0.0f };

    std::vector<int> outer(10);
    outer[0] = 0;
    outer[1] = 2;
    outer[2] = 4;
    outer[3] = 6;
    outer[4] = 8;
    outer[5] = 10;
    outer[6] = 12;
    outer[7] = 14;
    outer[8] = 16;
    outer[9] = 18;

    Triangulator::Polygon outerPoly = { (int)outer.size(), &outer[0] };

    mTriangulator = std::make_unique<Triangulator>(numPoints, &mPoints[0]);
    (*mTriangulator)(outerPoly);
    auto const& triangles = mTriangulator->GetAllTriangles();
    int numTriangles = (int)triangles.size();
    mPMesher = std::make_unique<PlanarMesher>(numPoints, &mPoints[0],
        numTriangles, (int const*)&triangles[0]);

    DrawTriangulation();
}

void TriangulationCDTWindow::OneNestedPolygon()
{
    int numPoints = 7;
    mPoints.resize(numPoints);
    mPoints[0] = { 128.0f, 256.0f };
    mPoints[1] = { 256.0f, 128.0f };
    mPoints[2] = { 384.0f, 256.0f };
    mPoints[3] = { 256.0f, 384.0f };
    mPoints[4] = { 320.0f, 256.0f };
    mPoints[5] = { 256.0f, 192.0f };
    mPoints[6] = { 256.0f, 320.0f };

    std::vector<int> outer(4);
    outer[0] = 0;
    outer[1] = 1;
    outer[2] = 2;
    outer[3] = 3;

    std::vector<int> inner(3);
    inner[0] = 4;
    inner[1] = 5;
    inner[2] = 6;

    Triangulator::Polygon outerPoly = { (int)outer.size(), &outer[0] };
    Triangulator::Polygon innerPoly = { (int)inner.size(), &inner[0] };

    mTriangulator = std::make_unique<Triangulator>(numPoints, &mPoints[0]);
    (*mTriangulator)(outerPoly, innerPoly);
    auto const& triangles = mTriangulator->GetAllTriangles();
    int numTriangles = (int)triangles.size();
    mPMesher = std::make_unique<PlanarMesher>(numPoints, &mPoints[0],
        numTriangles, (int const*)&triangles[0]);

    DrawTriangulation();
}

void TriangulationCDTWindow::TwoNestedPolygons()
{
    int numPoints = 16;
    mPoints.resize(numPoints);
    mPoints[0] = { 58.0f, 278.0f };
    mPoints[1] = { 156.0f, 198.0f };
    mPoints[2] = { 250.0f, 282.0f };
    mPoints[3] = { 328.0f, 232.0f };
    mPoints[4] = { 402.0f, 336.0f };
    mPoints[5] = { 314.0f, 326.0f };
    mPoints[6] = { 274.0f, 400.0f };
    mPoints[7] = { 196.0f, 268.0f };
    mPoints[8] = { 104.0f, 292.0f };
    mPoints[9] = { 110.0f, 382.0f };
    mPoints[10] = (mPoints[2] + mPoints[5] + mPoints[6]) / 3.0f;
    mPoints[11] = (mPoints[2] + mPoints[3] + mPoints[4]) / 3.0f;
    mPoints[12] = (mPoints[2] + mPoints[6] + mPoints[7]) / 3.0f;
    mPoints[13] = (mPoints[1] + mPoints[0] + mPoints[8]) / 3.0f;
    mPoints[14] = (mPoints[1] + mPoints[8] + mPoints[7]) / 3.0f;
    mPoints[14][1] += 6.0f;
    mPoints[15] = (mPoints[1] + mPoints[7] + mPoints[2]) / 3.0f;

    std::vector<int> outer(10);
    outer[0] = 0;
    outer[1] = 1;
    outer[2] = 2;
    outer[3] = 3;
    outer[4] = 4;
    outer[5] = 5;
    outer[6] = 6;
    outer[7] = 7;
    outer[8] = 8;
    outer[9] = 9;

    std::vector<int> inner0(3);
    inner0[0] = 11;
    inner0[1] = 12;
    inner0[2] = 10;

    std::vector<int> inner1(3);
    inner1[0] = 13;
    inner1[1] = 14;
    inner1[2] = 15;

    Triangulator::Polygon outerPoly = { (int)outer.size(), &outer[0] };
    std::vector<Triangulator::Polygon> innerPolys(2);
    innerPolys[0] = { (int)inner0.size(), &inner0[0] };
    innerPolys[1] = { (int)inner1.size(), &inner1[0] };

    mTriangulator = std::make_unique<Triangulator>(numPoints, &mPoints[0]);
    (*mTriangulator)(outerPoly, innerPolys);
    auto const& triangles = mTriangulator->GetAllTriangles();
    int numTriangles = (int)triangles.size();
    mPMesher = std::make_unique<PlanarMesher>(numPoints, &mPoints[0],
        numTriangles, (int const*)&triangles[0]);

    DrawTriangulation();
}

void TriangulationCDTWindow::TreeOfNestedPolygons()
{
    int numPoints = 43;
    mPoints.resize(numPoints);
    mPoints[0] = { 204.0f, 30.0f };
    mPoints[1] = { 466.0f, 174.0f };
    mPoints[2] = { 368.0f, 496.0f };
    mPoints[3] = { 66.0f, 464.0f };
    mPoints[4] = { 28.0f, 256.0f };
    mPoints[5] = { 274.0f, 84.0f };
    mPoints[6] = { 186.0f, 82.0f };
    mPoints[7] = { 274.0f, 158.0f };
    mPoints[8] = { 292.0f, 132.0f };
    mPoints[9] = { 322.0f, 426.0f };
    mPoints[10] = { 426.0f, 226.0f };
    mPoints[11] = { 216.0f, 134.0f };
    mPoints[12] = { 72.0f, 306.0f };
    mPoints[13] = { 178.0f, 440.0f };
    mPoints[14] = { 266.0f, 372.0f };
    mPoints[15] = { 294.0f, 474.0f };
    mPoints[16] = { 354.0f, 474.0f };
    mPoints[17] = { 368.0f, 404.0f };
    mPoints[18] = { 318.0f, 450.0f };
    mPoints[19] = { 172.0f, 226.0f };
    mPoints[20] = { 230.0f, 236.0f };
    mPoints[21] = { 196.0f, 268.0f };
    mPoints[22] = { 218.0f, 306.0f };
    mPoints[23] = { 136.0f, 266.0f };
    mPoints[24] = { 136.0f, 312.0f };
    mPoints[25] = { 230.0f, 350.0f };
    mPoints[26] = { 216.0f, 388.0f };
    mPoints[27] = { 160.0f, 384.0f };
    mPoints[28] = { 326.0f, 216.0f };
    mPoints[29] = { 370.0f, 216.0f };
    mPoints[30] = { 344.0f, 352.0f };
    mPoints[31] = { 158.0f, 340.0f };
    mPoints[32] = { 158.0f, 358.0f };
    mPoints[33] = { 176.0f, 358.0f };
    mPoints[34] = { 176.0f, 340.0f };
    mPoints[35] = { 192.0f, 358.0f };
    mPoints[36] = { 192.0f, 374.0f };
    mPoints[37] = { 206.0f, 374.0f };
    mPoints[38] = { 206.0f, 358.0f };
    mPoints[39] = { 338.0f, 242.0f };
    mPoints[40] = { 338.0f, 262.0f };
    mPoints[41] = { 356.0f, 262.0f };
    mPoints[42] = { 356.0f, 242.0f };

    // The nested tree of polygons has the structure
    //
    // outer0
    //     inner0
    //     inner1
    //         outer3
    //             inner5
    //     inner2
    //         outer1
    //         outer2
    //             inner3
    //             inner4

    Triangulator::Tree* outer0 = new Triangulator::Tree();
    outer0->polygon.numIndices = 5;
    outer0->polygon.indices = new int[outer0->polygon.numIndices];
    outer0->polygon.indices[0] = 0;
    outer0->polygon.indices[1] = 1;
    outer0->polygon.indices[2] = 2;
    outer0->polygon.indices[3] = 3;
    outer0->polygon.indices[4] = 4;

    Triangulator::Tree* inner0 = new Triangulator::Tree();
    inner0->polygon.numIndices = 3;
    inner0->polygon.indices = new int[inner0->polygon.numIndices];
    inner0->polygon.indices[0] = 5;
    inner0->polygon.indices[1] = 6;
    inner0->polygon.indices[2] = 7;
    outer0->child.push_back(inner0);

    Triangulator::Tree* inner1 = new Triangulator::Tree();
    inner1->polygon.numIndices = 3;
    inner1->polygon.indices = new int[inner1->polygon.numIndices];
    inner1->polygon.indices[0] = 8;
    inner1->polygon.indices[1] = 9;
    inner1->polygon.indices[2] = 10;
    outer0->child.push_back(inner1);

    Triangulator::Tree* inner2 = new Triangulator::Tree();
    inner2->polygon.numIndices = 8;
    inner2->polygon.indices = new int[inner2->polygon.numIndices];
    inner2->polygon.indices[0] = 11;
    inner2->polygon.indices[1] = 12;
    inner2->polygon.indices[2] = 13;
    inner2->polygon.indices[3] = 14;
    inner2->polygon.indices[4] = 15;
    inner2->polygon.indices[5] = 16;
    inner2->polygon.indices[6] = 17;
    inner2->polygon.indices[7] = 18;
    outer0->child.push_back(inner2);

    Triangulator::Tree* outer1 = new Triangulator::Tree();
    outer1->polygon.numIndices = 5;
    outer1->polygon.indices = new int[outer1->polygon.numIndices];
    outer1->polygon.indices[0] = 19;
    outer1->polygon.indices[1] = 20;
    outer1->polygon.indices[2] = 21;
    outer1->polygon.indices[3] = 22;
    outer1->polygon.indices[4] = 23;
    inner2->child.push_back(outer1);

    Triangulator::Tree* outer2 = new Triangulator::Tree();
    outer2->polygon.numIndices = 4;
    outer2->polygon.indices = new int[outer2->polygon.numIndices];
    outer2->polygon.indices[0] = 24;
    outer2->polygon.indices[1] = 25;
    outer2->polygon.indices[2] = 26;
    outer2->polygon.indices[3] = 27;
    inner2->child.push_back(outer2);

    Triangulator::Tree* outer3 = new Triangulator::Tree();
    outer3->polygon.numIndices = 3;
    outer3->polygon.indices = new int[outer3->polygon.numIndices];
    outer3->polygon.indices[0] = 28;
    outer3->polygon.indices[1] = 29;
    outer3->polygon.indices[2] = 30;
    inner1->child.push_back(outer3);

    Triangulator::Tree* inner3 = new Triangulator::Tree();
    inner3->polygon.numIndices = 4;
    inner3->polygon.indices = new int[inner3->polygon.numIndices];
    inner3->polygon.indices[0] = 31;
    inner3->polygon.indices[1] = 32;
    inner3->polygon.indices[2] = 33;
    inner3->polygon.indices[3] = 34;
    outer2->child.push_back(inner3);

    Triangulator::Tree* inner4 = new Triangulator::Tree();
    inner4->polygon.numIndices = 4;
    inner4->polygon.indices = new int[inner4->polygon.numIndices];
    inner4->polygon.indices[0] = 35;
    inner4->polygon.indices[1] = 36;
    inner4->polygon.indices[2] = 37;
    inner4->polygon.indices[3] = 38;
    outer2->child.push_back(inner4);

    Triangulator::Tree* inner5 = new Triangulator::Tree();
    inner5->polygon.numIndices = 4;
    inner5->polygon.indices = new int[inner5->polygon.numIndices];
    inner5->polygon.indices[0] = 39;
    inner5->polygon.indices[1] = 40;
    inner5->polygon.indices[2] = 41;
    inner5->polygon.indices[3] = 42;
    outer3->child.push_back(inner5);

    mTriangulator = std::make_unique<Triangulator>(numPoints, &mPoints[0]);
    (*mTriangulator)(*outer0);
    auto const& triangles = mTriangulator->GetAllTriangles();
    int numTriangles = (int)triangles.size();
    mPMesher = std::make_unique<PlanarMesher>(numPoints, &mPoints[0],
        numTriangles, (int const*)&triangles[0]);

    DrawTriangulation();

    mTriangulator->Delete(outer0, true);
}

