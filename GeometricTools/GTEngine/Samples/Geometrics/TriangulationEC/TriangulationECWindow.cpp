// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "TriangulationECWindow.h"


TriangulationECWindow::TriangulationECWindow(Parameters& parameters)
    :
    Window(parameters),
    mTree(nullptr),
    mExample(0),
    mColor(0xFF000000),
    mBlack(0xFF000000),
    mBlue(0xFFFF8000),
    mWhite(0xFFFFFFFF)
{
    mScreenTexture = std::make_unique<Texture2>(DF_R8G8B8A8_UNORM, mXSize,
        mYSize);
    mScreenTexture->SetUsage(Resource::DYNAMIC_UPDATE);
    memset(mScreenTexture->GetData(), 0xFF, mScreenTexture->GetNumBytes());
    mScreenTexels = mScreenTexture->Get<unsigned int>();

    mOverlay = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, mXSize, mYSize, SamplerState::MIN_P_MAG_P_MIP_P,
        SamplerState::CLAMP, SamplerState::CLAMP, true);
    mOverlay->SetTexture(mScreenTexture);

    UnindexedSimplePolygon();
}

void TriangulationECWindow::OnDisplay()
{
    // Clear the screen to white.
    memset(mScreenTexture->GetData(), 0xFF, mScreenTexture->GetNumBytes());

    int i, i0, i1, numPositions, x0, y0, x1, y1;

    // Draw the polygon edges.
    mColor = mBlack;
    switch (mExample)
    {
    case 0:
        numPositions = (int)mPositions.size();
        for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
        {
            x0 = (int)mPositions[i0][0];
            y0 = (int)mPositions[i0][1];
            x1 = (int)mPositions[i1][0];
            y1 = (int)mPositions[i1][1];
            DrawLine(x0, y0, x1, y1);
        }
        break;
    case 1:
        numPositions = (int)mOuter.size();
        for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
        {
            x0 = (int)mPositions[mOuter[i0]][0];
            y0 = (int)mPositions[mOuter[i0]][1];
            x1 = (int)mPositions[mOuter[i1]][0];
            y1 = (int)mPositions[mOuter[i1]][1];
            DrawLine(x0, y0, x1, y1);
        }
        break;
    case 2:
        numPositions = (int)mOuter.size();
        for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
        {
            x0 = (int)mPositions[mOuter[i0]][0];
            y0 = (int)mPositions[mOuter[i0]][1];
            x1 = (int)mPositions[mOuter[i1]][0];
            y1 = (int)mPositions[mOuter[i1]][1];
            DrawLine(x0, y0, x1, y1);
        }

        numPositions = (int)mInner0.size();
        for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
        {
            x0 = (int)mPositions[mInner0[i0]][0];
            y0 = (int)mPositions[mInner0[i0]][1];
            x1 = (int)mPositions[mInner0[i1]][0];
            y1 = (int)mPositions[mInner0[i1]][1];
            DrawLine(x0, y0, x1, y1);
        }
        break;

    case 3:
        numPositions = (int)mOuter.size();
        for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
        {
            x0 = (int)mPositions[mOuter[i0]][0];
            y0 = (int)mPositions[mOuter[i0]][1];
            x1 = (int)mPositions[mOuter[i1]][0];
            y1 = (int)mPositions[mOuter[i1]][1];
            DrawLine(x0, y0, x1, y1);
        }

        numPositions = (int)mInner0.size();
        for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
        {
            x0 = (int)mPositions[mInner0[i0]][0];
            y0 = (int)mPositions[mInner0[i0]][1];
            x1 = (int)mPositions[mInner0[i1]][0];
            y1 = (int)mPositions[mInner0[i1]][1];
            DrawLine(x0, y0, x1, y1);
        }

        numPositions = (int)mInner1.size();
        for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
        {
            x0 = (int)mPositions[mInner1[i0]][0];
            y0 = (int)mPositions[mInner1[i0]][1];
            x1 = (int)mPositions[mInner1[i1]][0];
            y1 = (int)mPositions[mInner1[i1]][1];
            DrawLine(x0, y0, x1, y1);
        }
        break;

    case 4:
    {
        std::queue<Triangulator::Tree*> treeQueue;
        treeQueue.push(mTree);
        while (treeQueue.size() > 0)
        {
            Triangulator::Tree* tree = treeQueue.front();
            treeQueue.pop();
            numPositions = (int)tree->polygon.numIndices;
            for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
            {
                x0 = (int)mPositions[tree->polygon.indices[i0]][0];
                y0 = (int)mPositions[tree->polygon.indices[i0]][1];
                x1 = (int)mPositions[tree->polygon.indices[i1]][0];
                y1 = (int)mPositions[tree->polygon.indices[i1]][1];
                DrawLine(x0, y0, x1, y1);
            }

            for (i = 0; i < (int)tree->child.size(); ++i)
            {
                treeQueue.push(tree->child[i]);
            }
        }
        break;
    }

    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
        numPositions = (int)mOuter.size();
        for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
        {
            x0 = (int)mPositions[mOuter[i0]][0];
            y0 = (int)mPositions[mOuter[i0]][1];
            x1 = (int)mPositions[mOuter[i1]][0];
            y1 = (int)mPositions[mOuter[i1]][1];
            DrawLine(x0, y0, x1, y1);
        }

        numPositions = (int)mInner0.size();
        for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
        {
            x0 = (int)mPositions[mInner0[i0]][0];
            y0 = (int)mPositions[mInner0[i0]][1];
            x1 = (int)mPositions[mInner0[i1]][0];
            y1 = (int)mPositions[mInner0[i1]][1];
            DrawLine(x0, y0, x1, y1);
        }

        numPositions = (int)mInner1.size();
        for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
        {
            x0 = (int)mPositions[mInner1[i0]][0];
            y0 = (int)mPositions[mInner1[i0]][1];
            x1 = (int)mPositions[mInner1[i1]][0];
            y1 = (int)mPositions[mInner1[i1]][1];
            DrawLine(x0, y0, x1, y1);
        }

        numPositions = (int)mInner2.size();
        for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
        {
            x0 = (int)mPositions[mInner2[i0]][0];
            y0 = (int)mPositions[mInner2[i0]][1];
            x1 = (int)mPositions[mInner2[i1]][0];
            y1 = (int)mPositions[mInner2[i1]][1];
            DrawLine(x0, y0, x1, y1);
        }
        break;
    }

    // Flood fill the polygon inside.
    for (auto const& seed : mFillSeeds)
    {
        DrawFloodFill((int)seed[0], (int)seed[1]);
    }

    // Draw the triangulation edges.
    mColor = mBlack;
    for (auto const& tri : mTriangles)
    {
        int v0 = tri[0];
        int v1 = tri[1];
        int v2 = tri[2];

        x0 = (int)mPositions[v0][0];
        y0 = (int)mPositions[v0][1];
        x1 = (int)mPositions[v1][0];
        y1 = (int)mPositions[v1][1];
        DrawLine(x0, y0, x1, y1);

        x0 = (int)mPositions[v1][0];
        y0 = (int)mPositions[v1][1];
        x1 = (int)mPositions[v2][0];
        y1 = (int)mPositions[v2][1];
        DrawLine(x0, y0, x1, y1);

        x0 = (int)mPositions[v2][0];
        y0 = (int)mPositions[v2][1];
        x1 = (int)mPositions[v0][0];
        y1 = (int)mPositions[v0][1];
        DrawLine(x0, y0, x1, y1);
    }

    mEngine->Update(mScreenTexture);
    mEngine->Draw(mOverlay);
    mEngine->DisplayColorBuffer(0);
}

bool TriangulationECWindow::OnCharPress(unsigned char key, int x, int y)
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
    case '5':
        FourBoxesThreeNested(0, 1, 2);
        OnDisplay();
        return true;
    case '6':
        FourBoxesThreeNested(0, 2, 1);
        OnDisplay();
        return true;
    case '7':
        FourBoxesThreeNested(1, 0, 2);
        OnDisplay();
        return true;
    case '8':
        FourBoxesThreeNested(1, 2, 0);
        OnDisplay();
        return true;
    case '9':
        FourBoxesThreeNested(2, 0, 1);
        OnDisplay();
        return true;
    case 'a':
    case 'A':
        FourBoxesThreeNested(2, 1, 0);
        OnDisplay();
        return true;
    }

    return Window::OnCharPress(key, x, y);
}

void TriangulationECWindow::DrawLine(int x0, int y0, int x1, int y1)
{
    ImageUtility2::DrawLine(x0, y0, x1, y1,
        [this](int x, int y)
        {
            mScreenTexels[x + mXSize * (mYSize - 1 - y)] = mColor;
        }
    );
}

void TriangulationECWindow::DrawFloodFill(int x, int y)
{
    ImageUtility2::DrawFloodFill4<unsigned int>(
        x, y, mXSize, mYSize, mBlue, mWhite,

        [this](int x, int y, unsigned int color)
        {
            mScreenTexels[x + mXSize * (mYSize - 1 - y)] = color;
        },

        [this](int x, int y)
        {
            return mScreenTexels[x + mXSize * (mYSize - 1 - y)];
        }
    );
}

void TriangulationECWindow::ClearAll()
{
    mPositions.clear();
    mOuter.clear();
    mInner0.clear();
    mInner1.clear();
    mInner2.clear();
    Triangulator::Delete(mTree, true);
    mFillSeeds.clear();
    mTriangles.clear();
}

void TriangulationECWindow::UnindexedSimplePolygon()
{
    ClearAll();
    mExample = 0;

    mPositions.resize(10);
    mPositions[0] = { 58.0f, 278.0f };
    mPositions[1] = { 156.0f, 198.0f };
    mPositions[2] = { 250.0f, 282.0f };
    mPositions[3] = { 328.0f, 232.0f };
    mPositions[4] = { 402.0f, 336.0f };
    mPositions[5] = { 314.0f, 326.0f };
    mPositions[6] = { 274.0f, 400.0f };
    mPositions[7] = { 196.0f, 268.0f };
    mPositions[8] = { 104.0f, 292.0f };
    mPositions[9] = { 110.0f, 382.0f };

    mFillSeeds.push_back(Vector2<float>{ 132.0f, 256.0f });

    Triangulator triangulator((int)mPositions.size(), &mPositions[0]);
    triangulator();
    mTriangles = triangulator.GetTriangles();
}

void TriangulationECWindow::IndexedSimplePolygon()
{
    ClearAll();
    mExample = 1;

    mPositions.resize(20);
    mPositions[0] = { 58.0f, 278.0f };
    mPositions[1] = { 0.0f, 0.0f };
    mPositions[2] = { 156.0f, 198.0f };
    mPositions[3] = { 0.0f, 0.0f };
    mPositions[4] = { 250.0f, 282.0f };
    mPositions[5] = { 0.0f, 0.0f };
    mPositions[6] = { 328.0f, 232.0f };
    mPositions[7] = { 0.0f, 0.0f };
    mPositions[8] = { 402.0f, 336.0f };
    mPositions[9] = { 0.0f, 0.0f };
    mPositions[10] = { 314.0f, 326.0f };
    mPositions[11] = { 0.0f, 0.0f };
    mPositions[12] = { 274.0f, 400.0f };
    mPositions[13] = { 0.0f, 0.0f };
    mPositions[14] = { 196.0f, 268.0f };
    mPositions[15] = { 0.0f, 0.0f };
    mPositions[16] = { 104.0f, 292.0f };
    mPositions[17] = { 0.0f, 0.0f };
    mPositions[18] = { 110.0f, 382.0f };
    mPositions[19] = { 0.0f, 0.0f };

    mOuter.resize(10);
    mOuter[0] = 0;
    mOuter[1] = 2;
    mOuter[2] = 4;
    mOuter[3] = 6;
    mOuter[4] = 8;
    mOuter[5] = 10;
    mOuter[6] = 12;
    mOuter[7] = 14;
    mOuter[8] = 16;
    mOuter[9] = 18;

    mFillSeeds.push_back(Vector2<float>{ 132.0f, 256 });

    Triangulator::Polygon outer = { (int)mOuter.size(), &mOuter[0] };
    Triangulator triangulator((int)mPositions.size(), &mPositions[0]);
    triangulator(outer);
    mTriangles = triangulator.GetTriangles();
}

void TriangulationECWindow::OneNestedPolygon()
{
    // Polygon with one hole.  The top and bottom vertices of the outer
    // polygon are on the line containing the left edge of the inner polygon.
    // This example tests how the collinearity detection works when
    // identifying ears.

    ClearAll();
    mExample = 2;

    mPositions.resize(7);
    mPositions[0] = { 128.0f, 256.0f };
    mPositions[1] = { 256.0f, 128.0f };
    mPositions[2] = { 384.0f, 256.0f };
    mPositions[3] = { 256.0f, 384.0f };
    mPositions[4] = { 320.0f, 256.0f };
    mPositions[5] = { 256.0f, 192.0f };
    mPositions[6] = { 256.0f, 320.0f };

    mOuter.resize(4);
    mOuter[0] = 0;
    mOuter[1] = 1;
    mOuter[2] = 2;
    mOuter[3] = 3;
    mFillSeeds.push_back(Vector2<float>{ 132.0f, 256.0f });

    mInner0.resize(3);
    mInner0[0] = 4;
    mInner0[1] = 5;
    mInner0[2] = 6;

    Triangulator::Polygon outer = { (int)mOuter.size(), &mOuter[0] };
    Triangulator::Polygon inner = { (int)mInner0.size(), &mInner0[0] };
    Triangulator triangulator((int)mPositions.size(), &mPositions[0]);
    triangulator(outer, inner);
    mTriangles = triangulator.GetTriangles();
}

void TriangulationECWindow::TwoNestedPolygons()
{
    ClearAll();
    mExample = 3;

    mPositions.resize(16);
    mPositions[0] = { 58.0f, 278.0f };
    mPositions[1] = { 156.0f, 198.0f };
    mPositions[2] = { 250.0f, 282.0f };
    mPositions[3] = { 328.0f, 232.0f };
    mPositions[4] = { 402.0f, 336.0f };
    mPositions[5] = { 314.0f, 326.0f };
    mPositions[6] = { 274.0f, 400.0f };
    mPositions[7] = { 196.0f, 268.0f };
    mPositions[8] = { 104.0f, 292.0f };
    mPositions[9] = { 110.0f, 382.0f };
    mPositions[10] = (mPositions[2] + mPositions[5] + mPositions[6]) / 3.0f;
    mPositions[11] = (mPositions[2] + mPositions[3] + mPositions[4]) / 3.0f;
    mPositions[12] = (mPositions[2] + mPositions[6] + mPositions[7]) / 3.0f;
    mPositions[13] = (mPositions[1] + mPositions[0] + mPositions[8]) / 3.0f;
    mPositions[14] = (mPositions[1] + mPositions[8] + mPositions[7]) / 3.0f;
    mPositions[14][1] += 6.0f;
    mPositions[15] = (mPositions[1] + mPositions[7] + mPositions[2]) / 3.0f;

    mOuter.resize(10);
    mOuter[0] = 0;
    mOuter[1] = 1;
    mOuter[2] = 2;
    mOuter[3] = 3;
    mOuter[4] = 4;
    mOuter[5] = 5;
    mOuter[6] = 6;
    mOuter[7] = 7;
    mOuter[8] = 8;
    mOuter[9] = 9;
    mFillSeeds.push_back(Vector2<float>{ 62.0f, 278.0f });

    mInner0.resize(3);
    mInner0[0] = 11;
    mInner0[1] = 12;
    mInner0[2] = 10;

    mInner1.resize(3);
    mInner1[0] = 13;
    mInner1[1] = 14;
    mInner1[2] = 15;

    Triangulator::Polygon outer = { (int)mOuter.size(), &mOuter[0] };
    std::vector<Triangulator::Polygon> inners(2);
    inners[0] = { (int)mInner0.size(), &mInner0[0] };
    inners[1] = { (int)mInner1.size(), &mInner1[0] };
    Triangulator triangulator((int)mPositions.size(), &mPositions[0]);
    triangulator(outer, inners);
    mTriangles = triangulator.GetTriangles();
}

void TriangulationECWindow::TreeOfNestedPolygons()
{
    ClearAll();
    mExample = 4;
    mPositions.resize(43);
    mPositions[0] = { 204.0f, 30.0f };
    mPositions[1] = { 466.0f, 174.0f };
    mPositions[2] = { 368.0f, 496.0f };
    mPositions[3] = { 66.0f, 464.0f };
    mPositions[4] = { 28.0f, 256.0f };
    mPositions[5] = { 274.0f, 84.0f };
    mPositions[6] = { 186.0f, 82.0f };
    mPositions[7] = { 274.0f, 158.0f };
    mPositions[8] = { 292.0f, 132.0f };
    mPositions[9] = { 322.0f, 426.0f };
    mPositions[10] = { 426.0f, 226.0f };
    mPositions[11] = { 216.0f, 134.0f };
    mPositions[12] = { 72.0f, 306.0f };
    mPositions[13] = { 178.0f, 440.0f };
    mPositions[14] = { 266.0f, 372.0f };
    mPositions[15] = { 294.0f, 474.0f };
    mPositions[16] = { 354.0f, 474.0f };
    mPositions[17] = { 368.0f, 404.0f };
    mPositions[18] = { 318.0f, 450.0f };
    mPositions[19] = { 172.0f, 226.0f };
    mPositions[20] = { 230.0f, 236.0f };
    mPositions[21] = { 196.0f, 268.0f };
    mPositions[22] = { 218.0f, 306.0f };
    mPositions[23] = { 136.0f, 266.0f };
    mPositions[24] = { 136.0f, 312.0f };
    mPositions[25] = { 230.0f, 350.0f };
    mPositions[26] = { 216.0f, 388.0f };
    mPositions[27] = { 160.0f, 384.0f };
    mPositions[28] = { 326.0f, 216.0f };
    mPositions[29] = { 370.0f, 216.0f };
    mPositions[30] = { 344.0f, 352.0f };
    mPositions[31] = { 158.0f, 340.0f };
    mPositions[32] = { 158.0f, 358.0f };
    mPositions[33] = { 176.0f, 358.0f };
    mPositions[34] = { 176.0f, 340.0f };
    mPositions[35] = { 192.0f, 358.0f };
    mPositions[36] = { 192.0f, 374.0f };
    mPositions[37] = { 206.0f, 374.0f };
    mPositions[38] = { 206.0f, 358.0f };
    mPositions[39] = { 338.0f, 242.0f };
    mPositions[40] = { 338.0f, 262.0f };
    mPositions[41] = { 356.0f, 262.0f };
    mPositions[42] = { 356.0f, 242.0f };

    // outer0 polygon
    mTree = new Triangulator::Tree();
    mTree->polygon.numIndices = 5;
    mTree->polygon.indices = new int[mTree->polygon.numIndices];
    mTree->polygon.indices[0] = 0;
    mTree->polygon.indices[1] = 1;
    mTree->polygon.indices[2] = 2;
    mTree->polygon.indices[3] = 3;
    mTree->polygon.indices[4] = 4;
    mFillSeeds.push_back(Vector2<float>{ 164.0f, 138.0f });

    // inner0 polygon
    Triangulator::Tree* inner0 = new Triangulator::Tree();
    inner0->polygon.numIndices = 3;
    inner0->polygon.indices = new int[inner0->polygon.numIndices];
    inner0->polygon.indices[0] = 5;
    inner0->polygon.indices[1] = 6;
    inner0->polygon.indices[2] = 7;
    mTree->child.push_back(inner0);

    // inner1 polygon
    Triangulator::Tree* inner1 = new Triangulator::Tree();
    inner1->polygon.numIndices = 3;
    inner1->polygon.indices = new int[inner1->polygon.numIndices];
    inner1->polygon.indices[0] = 8;
    inner1->polygon.indices[1] = 9;
    inner1->polygon.indices[2] = 10;
    mTree->child.push_back(inner1);

    // inner2 polygon
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
    mTree->child.push_back(inner2);

    // outer1 polygon (contained in inner2)
    Triangulator::Tree* outer1 = new Triangulator::Tree();
    outer1->polygon.numIndices = 5;
    outer1->polygon.indices = new int[outer1->polygon.numIndices];
    outer1->polygon.indices[0] = 19;
    outer1->polygon.indices[1] = 20;
    outer1->polygon.indices[2] = 21;
    outer1->polygon.indices[3] = 22;
    outer1->polygon.indices[4] = 23;
    inner2->child.push_back(outer1);
    mFillSeeds.push_back(Vector2<float>{ 184.0f, 248.0f });

    // outer2 polygon (contained in inner2)
    Triangulator::Tree* outer2 = new Triangulator::Tree();
    outer2->polygon.numIndices = 4;
    outer2->polygon.indices = new int[outer2->polygon.numIndices];
    outer2->polygon.indices[0] = 24;
    outer2->polygon.indices[1] = 25;
    outer2->polygon.indices[2] = 26;
    outer2->polygon.indices[3] = 27;
    inner2->child.push_back(outer2);
    mFillSeeds.push_back(Vector2<float>{ 218.0f, 358.0f });

    // outer3 polygon (contained in inner1)
    Triangulator::Tree* outer3 = new Triangulator::Tree();
    outer3->polygon.numIndices = 3;
    outer3->polygon.indices = new int[outer3->polygon.numIndices];
    outer3->polygon.indices[0] = 28;
    outer3->polygon.indices[1] = 29;
    outer3->polygon.indices[2] = 30;
    inner1->child.push_back(outer3);
    mFillSeeds.push_back(Vector2<float>{ 344.0f, 278.0f });

    // inner3 polygon (contained in outer2)
    Triangulator::Tree* inner3 = new Triangulator::Tree();
    inner3->polygon.numIndices = 4;
    inner3->polygon.indices = new int[inner3->polygon.numIndices];
    inner3->polygon.indices[0] = 31;
    inner3->polygon.indices[1] = 32;
    inner3->polygon.indices[2] = 33;
    inner3->polygon.indices[3] = 34;
    outer2->child.push_back(inner3);

    // inner4 polygon (contained in outer2)
    Triangulator::Tree* inner4 = new Triangulator::Tree();
    inner4->polygon.numIndices = 4;
    inner4->polygon.indices = new int[inner4->polygon.numIndices];
    inner4->polygon.indices[0] = 35;
    inner4->polygon.indices[1] = 36;
    inner4->polygon.indices[2] = 37;
    inner4->polygon.indices[3] = 38;
    outer2->child.push_back(inner4);

    // inner5 polygon (contained in outer3)
    Triangulator::Tree* inner5 = new Triangulator::Tree();
    inner5->polygon.numIndices = 4;
    inner5->polygon.indices = new int[inner5->polygon.numIndices];
    inner5->polygon.indices[0] = 39;
    inner5->polygon.indices[1] = 40;
    inner5->polygon.indices[2] = 41;
    inner5->polygon.indices[3] = 42;
    outer3->child.push_back(inner5);

    Triangulator triangulator((int)mPositions.size(), &mPositions[0]);
    triangulator(*mTree);
    mTriangles = triangulator.GetTriangles();
}

void TriangulationECWindow::FourBoxesThreeNested(int i0, int i1, int i2)
{
    ClearAll();
    mExample = 5;

    mPositions.resize(16);
    mPositions[0] = { 64.0f, 16.0f };
    mPositions[1] = { 448.0f, 16.0f };
    mPositions[2] = { 448.0f, 496.0f };
    mPositions[3] = { 64.0f, 496.0f };
    mPositions[4] = { 320.0f, 32.0f };
    mPositions[5] = { 320.0f, 240.0f };
    mPositions[6] = { 384.0f, 240.0f };
    mPositions[7] = { 384.0f, 32.0f };
    mPositions[8] = { 320.0f, 272.0f };
    mPositions[9] = { 320.0f, 480.0f };
    mPositions[10] = { 384.0f, 480.0f };
    mPositions[11] = { 384.0f, 272.0f };
    mPositions[12] = { 128.0f, 272.0f };
    mPositions[13] = { 128.0f, 480.0f };
    mPositions[14] = { 192.0f, 480.0f };
    mPositions[15] = { 192.0f, 272.0f };

    mOuter.resize(4);
    mOuter[0] = 0;
    mOuter[1] = 1;
    mOuter[2] = 2;
    mOuter[3] = 3;
    mFillSeeds.push_back(Vector2<float>{ 128.0f, 32.0f });

    mInner0.resize(4);
    mInner0[0] = 4;
    mInner0[1] = 5;
    mInner0[2] = 6;
    mInner0[3] = 7;

    mInner1.resize(4);
    mInner1[0] = 8;
    mInner1[1] = 9;
    mInner1[2] = 10;
    mInner1[3] = 11;

    mInner2.resize(4);
    mInner2[0] = 12;
    mInner2[1] = 13;
    mInner2[2] = 14;
    mInner2[3] = 15;

    Triangulator::Polygon outer = { (int)mOuter.size(), &mOuter[0] };
    std::vector<Triangulator::Polygon> inners(3);
    inners[i0] = { (int)mInner0.size(), &mInner0[0] };
    inners[i1] = { (int)mInner1.size(), &mInner1[0] };
    inners[i2] = { (int)mInner2.size(), &mInner2[0] };
    Triangulator triangulator((int)mPositions.size(), &mPositions[0]);
    triangulator(outer, inners);
    mTriangles = triangulator.GetTriangles();
}

