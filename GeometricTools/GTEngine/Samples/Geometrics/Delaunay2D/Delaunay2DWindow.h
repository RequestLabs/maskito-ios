// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
using namespace gte;

class Delaunay2DWindow : public Window
{
public:
    Delaunay2DWindow(Parameters& parameters);

    virtual void OnDisplay();
    virtual bool OnMouseClick(MouseButton button, MouseState state,
        int x, int y, unsigned int modifiers);

private:
    void DrawLine(int x0, int y0, int x1, int y1, unsigned int color);
    void Fill(int x, int y, unsigned int foreColor, unsigned int backColor);

    Vector4<float> mTextColor;
    std::shared_ptr<OverlayEffect> mOverlay;
    std::shared_ptr<Texture2> mScreenTexture;
    unsigned int* mScreenTexels;

    std::vector<Vector2<float>> mVertices;
    std::vector<int> mHull;
    Delaunay2<float, BSNumber<UIntegerAP32>> mDelaunay;
    Delaunay2<float, BSNumber<UIntegerAP32>>::SearchInfo mInfo;
    int mCurrentTriX, mCurrentTriY, mCurrentIndex;
};
