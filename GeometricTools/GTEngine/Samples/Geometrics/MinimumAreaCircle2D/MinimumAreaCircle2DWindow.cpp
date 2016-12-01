// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "MinimumAreaCircle2DWindow.h"


MinimumAreaCircle2DWindow::~MinimumAreaCircle2DWindow()
{
}

MinimumAreaCircle2DWindow::MinimumAreaCircle2DWindow(Parameters& parameters)
    :
    Window(parameters),
    mNumActive(2),
    mVertices(NUM_POINTS)
{
    // Randomly generated points.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.25f*mXSize, 0.75f*mYSize);
    for (auto& v : mVertices)
    {
        v = { rnd(mte), rnd(mte) };
    }

    mMAC2(mNumActive, &mVertices[0], mMinimalCircle);

    mOverlay = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, mXSize, mYSize, SamplerState::MIN_P_MAG_P_MIP_P,
        SamplerState::CLAMP, SamplerState::CLAMP, true);

    mScreenTexture = std::make_shared<Texture2>(DF_R8G8B8A8_UNORM, mXSize,
        mYSize);
    mScreenTexture->SetUsage(Resource::DYNAMIC_UPDATE);
    mOverlay->SetTexture(mScreenTexture);
    mScreenTexels = mScreenTexture->Get<unsigned int>();
}

void MinimumAreaCircle2DWindow::OnDisplay()
{
    unsigned int gray = 0xFF808080;
    unsigned int blue = 0xFFFF0000;
    unsigned int red = 0xFF0000FF;

    // Clear the screen to white.
    memset(mScreenTexture->GetData(), 0xFF, mScreenTexture->GetNumBytes());

    // Draw the minimum area circle.
    int x = static_cast<int>(mMinimalCircle.center[0] + 0.5f);
    int y = static_cast<int>(mMinimalCircle.center[1] + 0.5f);
    int radius = static_cast<int>(mMinimalCircle.radius + 0.5f);
    DrawCircle(x, y, radius, gray);

    // Draw the support.
    int numSupport = mMAC2.GetNumSupport();
    std::array<int, 3> support = mMAC2.GetSupport();
    for (int i0 = numSupport - 1, i1 = 0; i1 <numSupport; i0 = i1++)
    {
        int x0 = static_cast<int>(mVertices[support[i0]][0] + 0.5f);
        int y0 = static_cast<int>(mVertices[support[i0]][1] + 0.5f);
        int x1 = static_cast<int>(mVertices[support[i1]][0] + 0.5f);
        int y1 = static_cast<int>(mVertices[support[i1]][1] + 0.5f);
        DrawLine(x0, y0, x1, y1, red);
    }

    // Draw the input points.
    for (int i = 0; i < mNumActive; ++i)
    {
        x = static_cast<int>(mVertices[i][0] + 0.5f);
        y = static_cast<int>(mVertices[i][1] + 0.5f);
        DrawPoint(x, y, blue);
    }

    mEngine->Update(mScreenTexture);
    mEngine->Draw(mOverlay);
    mEngine->DisplayColorBuffer(0);
}

bool MinimumAreaCircle2DWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'n':
    case 'N':
        if (mNumActive < NUM_POINTS)
        {
            mMAC2(++mNumActive, &mVertices[0], mMinimalCircle);
            OnDisplay();
        }
        return true;
    }

    return Window::OnCharPress(key, x, y);
}

void MinimumAreaCircle2DWindow::DrawCircle(int x, int y, int radius,
    unsigned int color)
{
    ImageUtility2::DrawCircle(x, y, radius, false,
        [this, color](int x, int y)
        {
            if (0 <= x && x < mXSize && 0 <= y && y < mYSize)
            {
                mScreenTexels[x + mXSize*y] = color;
            }
        }
    );
}

void MinimumAreaCircle2DWindow::DrawPoint(int x, int y, unsigned int color)
{
    for (int dy = -1; dy <= 1; ++dy)
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            mScreenTexels[(x + dx) + mXSize*(y + dy)] = color;
        }
    }
}

void MinimumAreaCircle2DWindow::DrawLine(int x0, int y0, int x1, int y1,
    unsigned int color)
{
    ImageUtility2::DrawLine(x0, y0, x1, y1,
        [this, color](int x, int y)
        {
            mScreenTexels[x + mXSize*y] = color;
        }
    );
}

