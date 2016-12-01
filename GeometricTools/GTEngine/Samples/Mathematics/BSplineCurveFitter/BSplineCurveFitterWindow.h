// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
using namespace gte;

class BSplineCurveFitterWindow : public Window3
{
public:
    BSplineCurveFitterWindow(Parameters& parameters);

    virtual void OnDisplay();
    virtual void OnIdle();

    virtual bool OnCharPress(unsigned char key, int x, int y);

    virtual bool OnMouseClick(MouseButton button, MouseState state,
        int x, int y, unsigned int modifiers);

    virtual bool OnMouseMotion(MouseButton button, int x, int y,
        unsigned int modifiers);

private:
    void CreateScene();
    void InitializeCamera();
    void CreateBSplinePolyline();

    enum { NUM_SAMPLES = 1000 };

    struct Vertex
    {
        Vector3<float> position;
        Vector4<float> color;
    };

    std::vector<Vector3<float>> mSamples;
    std::shared_ptr<Visual> mHelix, mPolyline;

    int mDegree, mNumControls;
    BSplineCurveFit<float>* mSpline;
    float mAvrError, mRmsError;
    std::string mMessage;
};
