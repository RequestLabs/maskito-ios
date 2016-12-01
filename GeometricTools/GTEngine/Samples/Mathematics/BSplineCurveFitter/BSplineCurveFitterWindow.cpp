// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "BSplineCurveFitterWindow.h"


BSplineCurveFitterWindow::BSplineCurveFitterWindow(Parameters& parameters)
    :
    Window3(parameters),
    mSamples(NUM_SAMPLES),
    mDegree(3),
    mNumControls(NUM_SAMPLES / 2),
    mSpline(nullptr),
    mAvrError(0.0f),
    mRmsError(0.0f)
{
    InitializeCamera();
    CreateScene();
}

void BSplineCurveFitterWindow::OnDisplay()
{
    mCameraRig.Move();

    mEngine->ClearBuffers();
    mEngine->Draw(mHelix);
    mEngine->Draw(mPolyline);

    Vector4<float> textColor{ 0.0f, 0.0f, 0.0f, 1.0f };
    mMessage = "samples = " + std::to_string(mSamples.size()) +
        ", degree = " + std::to_string(mDegree) +
        ", controls = " + std::to_string(mNumControls);
    mEngine->Draw(8, 16, textColor, mMessage);

    mMessage = "avr error = " + std::to_string(mAvrError) +
        ", rms error = " + std::to_string(mRmsError);
    mEngine->Draw(8, 36, textColor, mMessage);

    mEngine->DisplayColorBuffer(0);
}

void BSplineCurveFitterWindow::OnIdle()
{
    if (mCameraRig.Move())
    {
        OnDisplay();
    }
}

bool BSplineCurveFitterWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'd':  // reduce the degree
        if (mDegree > 1)
        {
            --mDegree;
            CreateBSplinePolyline();
            OnDisplay();
        }
        return true;

    case 'D':  // increase the degree
        ++mDegree;
        CreateBSplinePolyline();
        OnDisplay();
        return true;

    case 's':  // (small) reduce the number of control points by 1
        if (mNumControls - 1 > mDegree + 1)
        {
            --mNumControls;
            CreateBSplinePolyline();
            OnDisplay();
        }
        return true;

    case 'S':  // (small) increase the number of control points by 1
        if (mNumControls + 1 < NUM_SAMPLES)
        {
            ++mNumControls;
            CreateBSplinePolyline();
            OnDisplay();
        }
        return true;

    case 'm':  // (medium) reduce the number of control points by 10
        if (mNumControls - 10 > mDegree + 1)
        {
            mNumControls -= 10;
            CreateBSplinePolyline();
            OnDisplay();
        }
        return true;

    case 'M':  // (medium) increase the number of control points by 10
        if (mNumControls + 10 < NUM_SAMPLES)
        {
            mNumControls += 10;
            CreateBSplinePolyline();
            OnDisplay();
        }
        return true;

    case 'l':  // (large) reduce the number of control points by 100
        if (mNumControls - 100 > mDegree + 1)
        {
            mNumControls -= 100;
            CreateBSplinePolyline();
            OnDisplay();
        }
        return true;

    case 'L':  // (large) increase the number of control points by 100
        if (mNumControls + 100 < NUM_SAMPLES)
        {
            mNumControls += 100;
            CreateBSplinePolyline();
            OnDisplay();
        }
        return true;
    }

    return Window::OnCharPress(key, x, y);
}

bool BSplineCurveFitterWindow::OnMouseClick(MouseButton button,
    MouseState state, int x, int y, unsigned int modifiers)
{
    if (Window3::OnMouseClick(button, state, x, y, modifiers))
    {
        OnDisplay();
    }
    return true;
}

bool BSplineCurveFitterWindow::OnMouseMotion(MouseButton button, int x, int y,
    unsigned int modifiers)
{
    if (Window3::OnMouseMotion(button, x, y, modifiers))
    {
        OnDisplay();
    }
    return true;
}

void BSplineCurveFitterWindow::CreateScene()
{
    // Generate samples on a helix.
    unsigned int numSamples = (unsigned int)mSamples.size();
    float multiplier = 2.0f / (numSamples - 1.0f);
    for (unsigned int i = 0; i < numSamples; ++i)
    {
        float t = -1.0f + multiplier * i;
        float angle = 2.0f * (float)GTE_C_TWO_PI * t;
        float amplitude = 1.0f - t * t;
        mSamples[i][0] = amplitude * cos(angle);
        mSamples[i][1] = amplitude * sin(angle);
        mSamples[i][2] = t;
        Normalize(mSamples[i]);
    }

    // Create a graphics representation of the helix.  The colors are
    // randomly generated.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.25f, 0.75f);

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);
    std::shared_ptr<VertexBuffer> vbuffer(new VertexBuffer(vformat,
        numSamples));
    Vertex* vertex = vbuffer->Get<Vertex>();
    for (unsigned int i = 0; i < numSamples; ++i)
    {
        vertex[i].position = mSamples[i];
        vertex[i].color = { rnd(mte), rnd(mte), rnd(mte), 1.0f };
    }

    std::shared_ptr<IndexBuffer> ibuffer(new IndexBuffer(
        IP_POLYSEGMENT_CONTIGUOUS, numSamples - 1));

    std::shared_ptr<VertexColorEffect> effect =
        std::make_shared<VertexColorEffect>(mProgramFactory);

    mHelix = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mCameraRig.Subscribe(mHelix->worldTransform,
        effect->GetPVWMatrixConstant());

    mTrackball.Attach(mHelix);

    CreateBSplinePolyline();
}

void BSplineCurveFitterWindow::InitializeCamera()
{
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 0.1f, 100.0f);
    Vector4<float> camPosition{ 0.0f, 0.0f, -4.0f, 1.0f };
    Vector4<float> camDVector{ 0.0f, 0.0f, 1.0f, 0.0f };
    Vector4<float> camUVector{ 0.0f, 1.0f, 0.0f, 0.0f };
    Vector4<float> camRVector = Cross(camDVector, camUVector);
    mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

    mTranslationSpeed = 0.01f;
    mRotationSpeed = 0.001f;
    mTranslationSpeedFactor = 2.0f;
    mRotationSpeedFactor = 2.0f;
    mCameraRig.SetTranslationSpeed(mTranslationSpeed);
    mCameraRig.SetRotationSpeed(mRotationSpeed);
    mCameraRig.ComputeWorldAxes();
    mCameraRig.UpdatePVWMatrices();
}

void BSplineCurveFitterWindow::CreateBSplinePolyline()
{
    delete mSpline;
    if (mPolyline)
    {
        mTrackball.Detach(mPolyline);
        mCameraRig.Unsubscribe(mPolyline->worldTransform);
        mPolyline = nullptr;
    }

    // Create the curve from the current parameters.
    mSpline = new BSplineCurveFit<float>(3, (int)mSamples.size(),
        (float const*)&mSamples[0], mDegree, mNumControls);

    // Sample it the same number of times as the original data.
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);

    unsigned int numSamples = (unsigned int)mSamples.size();
    float multiplier = 1.0f / (numSamples - 1.0f);
    std::shared_ptr<VertexBuffer> vbuffer(new VertexBuffer(vformat,
        numSamples));
    Vertex* vertex = vbuffer->Get<Vertex>();
    for (unsigned int i = 0; i < numSamples; ++i)
    {
        float t = multiplier * i;
        mSpline->GetPosition(t, (float*)&vertex[i].position[0]);
        vertex[i].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    }

    std::shared_ptr<IndexBuffer> ibuffer(new IndexBuffer(
        IP_POLYSEGMENT_CONTIGUOUS, numSamples - 1));

    std::shared_ptr<VertexColorEffect> effect =
        std::make_shared<VertexColorEffect>(mProgramFactory);

    mPolyline = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mCameraRig.Subscribe(mPolyline->worldTransform,
        effect->GetPVWMatrixConstant());

    // Compute error measurements.
    mAvrError = 0.0f;
    mRmsError = 0.0f;
    for (unsigned int i = 0; i < numSamples; ++i)
    {
        Vector3<float> diff = mSamples[i] - vertex[i].position;
        float sqrLength = Dot(diff, diff);
        mRmsError += sqrLength;
        float length = sqrt(sqrLength);
        mAvrError += length;
    }
    mAvrError /= (float)numSamples;
    mRmsError /= (float)numSamples;
    mRmsError = sqrt(mRmsError);

    mTrackball.Attach(mPolyline);
    mTrackball.Update();
    mCameraRig.UpdatePVWMatrices();
}

