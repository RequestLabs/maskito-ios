// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "RopeWindow.h"


RopeWindow::RopeWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mWireState = std::make_shared<RasterizerState>();
    mWireState->fillMode = RasterizerState::FILL_WIREFRAME;
    mEngine->SetClearColor({ 0.75f, 0.85f, 0.95f, 1.0f });

    CreateSprings();
    CreateRope();
    InitializeCamera();

    mTime0 = std::chrono::high_resolution_clock::now();
}

void RopeWindow::OnIdle()
{
    mTimer.Measure();
    mCameraRig.Move();

    // Clamp to 120 frames per second.
    mTime1 = std::chrono::high_resolution_clock::now();
    int64_t delta = std::chrono::duration_cast<std::chrono::milliseconds>(
        mTime1 - mTime0).count();

    if (120 * delta >= 1000)
    {
        mTime0 = mTime1;
        PhysicsTick();
        GraphicsTick();
    }

    mTimer.UpdateFrameCount();
}

bool RopeWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':  // toggle wireframe
    case 'W':
        if (mWireState != mEngine->GetRasterizerState())
        {
            mEngine->SetRasterizerState(mWireState);
        }
        else
        {
            mEngine->SetDefaultRasterizerState();
        }
        return true;
    case 'm':  // decrease mass
        if (mModule->GetMass(1) > 0.05f)
        {
            for (int i = 1; i < mModule->GetNumParticles() - 1; ++i)
            {
                mModule->SetMass(i, mModule->GetMass(i) - 0.01f);
            }
        }
        return true;
    case 'M':  // increase mass
        for (int i = 1; i < mModule->GetNumParticles() - 1; ++i)
        {
            mModule->SetMass(i, mModule->GetMass(i) + 0.01f);
        }
        return true;
    case 'c':  // decrease spring constant
        if (mModule->GetConstant(0) > 0.05f)
        {
            for (int i = 0; i < mModule->GetNumSprings(); ++i)
            {
                mModule->SetConstant(i, mModule->GetConstant(i) - 0.01f);
            }
        }
        return true;
    case 'C':  // increase spring constant
        for (int i = 0; i < mModule->GetNumSprings(); ++i)
        {
            mModule->SetConstant(i, mModule->GetConstant(i) + 0.01f);
        }
        return true;
    case 'l':  // decrease spring resting length
        if (mModule->GetLength(0) > 0.05f)
        {
            for (int i = 0; i < mModule->GetNumSprings(); ++i)
            {
                mModule->SetLength(i, mModule->GetLength(i) - 0.01f);
            }
        }
        return true;
    case 'L':  // increase spring resting length
        for (int i = 0; i < mModule->GetNumSprings(); ++i)
        {
            mModule->SetLength(i, mModule->GetLength(i) + 0.01f);
        }
        return true;
    case 'f':  // toggle wind force on/off
    case 'F':
        mModule->enableWind = !mModule->enableWind;
        return true;
    case 'r':  // toggle random wind direction change on/off
    case 'R':
        mModule->enableWindChange = !mModule->enableWindChange;
        return true;
    }

    return Window::OnCharPress(key, x, y);
}

bool RopeWindow::SetEnvironment()
{
    std::string path = mEnvironment.GetVariable("GTE_PATH");
    if (path == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return false;
    }
    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("Rope.png") == "")
    {
        LogError("Cannot find file Rope.png.");
        return false;
    }

    return true;
}

void RopeWindow::CreateSprings()
{
    int numParticles = 8;
    float step = 0.1f;
    Vector3<float> gravity{ 0.0f, 0.0f, -1.0f };
    Vector3<float> wind{ 0.0f, -0.25f, 0.0f };
    float windChangeAmplitude = 0.01f;
    float viscosity = 10.0f;
    mModule = std::make_unique<PhysicsModule>(numParticles, step, gravity,
        wind, windChangeAmplitude, viscosity);

    // Constant mass at interior points (endpoints are immovable).
    mModule->SetMass(0, std::numeric_limits<float>::max());
    mModule->SetMass(numParticles - 1, std::numeric_limits<float>::max());
    for (int i = 1; i < numParticles - 1; ++i)
    {
        mModule->SetMass(i, 1.0f);
    }

    // Initial position on a horizontal line segment.
    float factor = 1.0f / (float)(numParticles - 1);
    for (int i = 0; i < numParticles; ++i)
    {
        mModule->SetPosition(i, { i * factor, 0.0f, 1.0f });
    }

    // Initial velocities are all zero.
    for (int i = 0; i < numParticles; ++i)
    {
        mModule->SetVelocity(i, Vector3<float>::Zero());
    }

    // Springs are at rest in the initial horizontal configuration.
    int numSprings = numParticles - 1;
    float restLength = 1.0f / (float)numSprings;
    for (int i = 0; i < numSprings; ++i)
    {
        mModule->SetConstant(i, 10.0f);
        mModule->SetLength(i, restLength);
    }
}

void RopeWindow::CreateRope()
{
    BasisFunctionInput<float> input;
    input.numControls = mModule->GetNumParticles();
    input.degree = 2;
    input.uniform = true;
    input.periodic = false;
    input.numUniqueKnots = input.numControls - input.degree + 1;
    std::vector<UniqueKnot<float>> uniqueKnots(input.numUniqueKnots);
    input.uniqueKnots = &uniqueKnots[0];
    uniqueKnots.front().t = 0.0f;
    uniqueKnots.front().multiplicity = input.degree + 1;
    for (int i = 1; i <= input.numUniqueKnots - 2; ++i)
    {
        uniqueKnots[i].t = i / (input.numUniqueKnots - 1.0f);
        uniqueKnots[i].multiplicity = 1;
    }
    uniqueKnots.back().t = 1.0f;
    uniqueKnots.back().multiplicity = input.degree + 1;

    mSpline = std::make_shared<BSplineCurve<3, float>>(input,
        &mModule->GetPosition(0));

    mSurface.reset(new TubeSurface<float>(mSpline,
        [](float){ return 0.025f; }));
    mSurface->Set(64, 8, false, false, true);
    int numVertices = mSurface->GetNumVertices();
    int numTriangles = mSurface->GetNumTriangles();

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
    std::shared_ptr<VertexBuffer> vbuffer(new VertexBuffer(vformat,
        numVertices));
    vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    Vertex* vertices = vbuffer->Get<Vertex>();
    mSurface->GetPositions(&vertices[0].position, sizeof(Vertex));
    mSurface->GetTCoords(&vertices[0].tcoord, sizeof(Vertex));

    std::shared_ptr<IndexBuffer> ibuffer(new IndexBuffer(IP_TRIMESH,
        numTriangles, sizeof(int)));
    mSurface->GetIndices(ibuffer->Get<int>());

    std::string path = mEnvironment.GetPath("Rope.png");
    std::shared_ptr<Texture2> texture(WICFileIO::Load(path, true));
    texture->AutogenerateMipmaps();
    std::shared_ptr<Texture2Effect> effect =
        std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::WRAP,
        SamplerState::WRAP);

    mRope = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mRope->UpdateModelBound();
    mRope->localTransform.SetTranslation(-mRope->modelBound.GetCenter());
    mCameraRig.Subscribe(mRope->worldTransform,
        effect->GetPVWMatrixConstant());

    mTrackball.Attach(mRope);
    mTrackball.Update();
}

void RopeWindow::InitializeCamera()
{
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 0.1f, 1000.0f);
    Vector4<float> camPosition{ 0.0f, 1.25f, -0.5f, 1.0f };
    Vector4<float> camDVector{ 0.0f, -1.0f, 0.0f, 0.0f };
    Vector4<float> camUVector{ 0.0f, 0.0f, 1.0f, 0.0f };
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

void RopeWindow::PhysicsTick()
{
    // Forces are independent of time, just pass in t = 0.
    mModule->Update(0.0f);

    // Update spline curve.  Remember that the spline maintains its own copy
    // of the control points, so this update is necessary.
    int numControls = mModule->GetNumParticles();
    for (int i = 0; i < numControls; ++i)
    {
        mSpline->SetControl(i, mModule->GetPosition(i));
    }

    // Update the GPU copy of the vertices.
    std::shared_ptr<VertexBuffer> vbuffer = mRope->GetVertexBuffer();
    Vertex* vertices = vbuffer->Get<Vertex>();
    mSurface->GetPositions(&vertices[0].position, sizeof(Vertex));
    mEngine->Update(vbuffer);
    mCameraRig.UpdatePVWMatrices();
}

void RopeWindow::GraphicsTick()
{
    mEngine->ClearBuffers();
    mEngine->Draw(mRope);
    mEngine->DisplayColorBuffer(0);
}

