// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "ConvexHull2DWindow.h"


ConvexHull2DWindow::ConvexHull2DWindow(Parameters& parameters)
    :
    Window(parameters)
{
#if 1
    // Randomly generated points.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.125f, 0.875f);
    mVertices.resize(256);
    for (auto& v : mVertices)
    {
        v[0] = mXSize*rnd(mte);
        v[1] = mYSize*rnd(mte);
    }
#endif

#if 0
    // A 3x3 square grid.
    mVertices.resize(9);
    mVertices[0] = Vector2<float>(64.0f, 64.0f);
    mVertices[1] = Vector2<float>(64.0f, 256.0f);
    mVertices[2] = Vector2<float>(64.0f, 448.0f);
    mVertices[3] = Vector2<float>(256.0f, 64.0f);
    mVertices[4] = Vector2<float>(256.0f, 256.0f);
    mVertices[5] = Vector2<float>(256.0f, 448.0f);
    mVertices[6] = Vector2<float>(448.0f, 64.0f);
    mVertices[7] = Vector2<float>(448.0f, 256.0f);
    mVertices[8] = Vector2<float>(448.0f, 448.0f);
#endif

    if (!mConvexHull(static_cast<int>(mVertices.size()), &mVertices[0],
        0.001f))
    {
        LogError("Degenerate point set.");
        parameters.created = false;
        return;
    }

    mOverlay = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, mXSize, mYSize, SamplerState::MIN_P_MAG_P_MIP_P,
        SamplerState::CLAMP, SamplerState::CLAMP, true);

    mScreenTexture = std::make_shared<Texture2>(DF_R8G8B8A8_UNORM, mXSize,
        mYSize);
    mScreenTexture->SetUsage(Resource::DYNAMIC_UPDATE);
    mOverlay->SetTexture(mScreenTexture);
    mScreenTexels = mScreenTexture->Get<unsigned int>();
}

void ConvexHull2DWindow::OnDisplay()
{
    unsigned int black = 0xFF000000;
    unsigned int gray = 0xFF808080;
    unsigned int blue = 0xFFFF0000;

    int x0, y0, x1, y1;
    Vector2<float> v0, v1;

    // Clear the screen to white.
    memset(mScreenTexture->GetData(), 0xFF, mScreenTexture->GetNumBytes());

    // Draw the convex polygon.
    std::vector<int> const& hull = mConvexHull.GetHull();
    int const numIndices = static_cast<int>(hull.size());
    for (int i0 = numIndices - 1, i1 = 0; i1 < numIndices; i0 = i1++)
    {
        v0 = mVertices[hull[i0]];
        x0 = static_cast<int>(v0[0] + 0.5f);
        y0 = static_cast<int>(v0[1] + 0.5f);

        v1 = mVertices[hull[i1]];
        x1 = static_cast<int>(v1[0] + 0.5f);
        y1 = static_cast<int>(v1[1] + 0.5f);

        DrawLine(x0, y0, x1, y1, gray);
    }

    // Draw the input points.
    for (auto const& v : mVertices)
    {
        x0 = static_cast<int>(v[0] + 0.5f);
        y0 = static_cast<int>(v[1] + 0.5f);
        DrawPoint(x0, y0, blue);
    }

    // Draw the hull points.
    for (auto index : hull)
    {
        v0 = mVertices[index];
        x0 = static_cast<int>(v0[0] + 0.5f);
        y0 = static_cast<int>(v0[1] + 0.5f);
        DrawPoint(x0, y0, black);
    }

    mEngine->Update(mScreenTexture);
    mEngine->Draw(mOverlay);
    mEngine->DisplayColorBuffer(0);
}

void ConvexHull2DWindow::DrawLine(int x0, int y0, int x1, int y1,
    unsigned int color)
{
    ImageUtility2::DrawLine(x0, y0, x1, y1,
        [this, color](int x, int y)
        {
            mScreenTexels[x + mXSize*y] = color;
        }
    );
}

void ConvexHull2DWindow::DrawPoint(int x, int y, unsigned int color)
{
    for (int dy = -1; dy <= 1; ++dy)
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            mScreenTexels[(x + dx) + mXSize*(y + dy)] = color;
        }
    }
}

