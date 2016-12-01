// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "ClothWindow.h"

//#define SINGLE_STEP


ClothWindow::ClothWindow(Parameters& parameters)
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
    mEngine->SetClearColor({ 0.85f, 0.85f, 1.0f, 1.0f });

    CreateSprings();
    CreateCloth();
    InitializeCamera();

    mTime0 = std::chrono::high_resolution_clock::now();
}

void ClothWindow::OnIdle()
{
    mTimer.Measure();
    mCameraRig.Move();

#if !defined(SINGLE_STEP)
    PhysicsTick();
#endif

    GraphicsTick();

    mTimer.UpdateFrameCount();
}

bool ClothWindow::OnCharPress(unsigned char key, int x, int y)
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

#if defined(SINGLE_STEP)
    case 'g':
    case 'G':
        PhysicsTick();
        return true;
#endif
    }

    return Window::OnCharPress(key, x, y);
}

bool ClothWindow::SetEnvironment()
{
    std::string path = mEnvironment.GetVariable("GTE_PATH");
    if (path == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return false;
    }
    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("Cloth.png") == "")
    {
        LogError("Cannot find file Cloth.png.");
        return false;
    }

    return true;
}

void ClothWindow::InitializeCamera()
{
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 0.1f, 100.0f);
    Vector4<float> camPosition{ 0.0f, -1.75f, 0.0f, 1.0f };
    Vector4<float> camDVector{ 0.0f, 1.0f, 0.0f, 0.0f };
    Vector4<float> camUVector{ 0.0f, 0.0f, 1.0f, 0.0f };
    Vector4<float> camRVector = Cross(camDVector, camUVector);
    mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

    mTranslationSpeed = 0.01f;
    mRotationSpeed = 0.01f;
    mTranslationSpeedFactor = 2.0f;
    mRotationSpeedFactor = 2.0f;
    mCameraRig.SetTranslationSpeed(mTranslationSpeed);
    mCameraRig.SetRotationSpeed(mRotationSpeed);
    mCameraRig.ComputeWorldAxes();
    mCameraRig.UpdatePVWMatrices();
}

void ClothWindow::CreateSprings()
{
    // Set up the mass-spring system.
    int numRows = 8;
    int numCols = 16;
    float step = 0.01f;
    Vector3<float> gravity{ 0.0f, 0.0f, -1.0f };
    Vector3<float> wind{ 0.5f, 0.0f, 0.0f };
    float viscosity = 10.0f;
    float maxAmplitude = 2.0f;
    mModule = std::make_unique<PhysicsModule>(numRows, numCols, step, gravity,
        wind, viscosity, maxAmplitude);

    // The top r of the mesh is immovable (infinite mass).  All other masses
    // are constant.
    int r, c;
    for (c = 0; c < numCols; ++c)
    {
        mModule->SetMass(numRows - 1, c, std::numeric_limits<float>::max());
    }
    for (r = 0; r < numRows - 1; ++r)
    {
        for (c = 0; c < numCols; ++c)
        {
            mModule->SetMass(r, c, 1.0f);
        }
    }

    // Initial position on a vertical axis-aligned rectangle, zero velocity.
    float rowFactor = 1.0f / (float)(numRows - 1);
    float colFactor = 1.0f / (float)(numCols - 1);
    for (r = 0; r < numRows; ++r)
    {
        for (c = 0; c < numCols; ++c)
        {
            float x = c*colFactor;
            float z = r*rowFactor;
            mModule->SetPosition(r, c, { x, 0.0f, z });
            mModule->SetVelocity(r, c, { 0.0f, 0.0f, 0.0f });
        }
    }

    // Springs are at rest in the initial configuration.
    float rowConstant = 1000.0f;
    float bottomConstant = 100.0f;
    Vector3<float> diff;
    for (r = 0; r < numRows; ++r)
    {
        for (c = 0; c < numCols - 1; ++c)
        {
            mModule->SetConstantC(r, c, rowConstant);
            diff = mModule->GetPosition(r, c + 1) -
                mModule->GetPosition(r, c);
            mModule->SetLengthC(r, c, Length(diff));
        }
    }

    for (r = 0; r < numRows - 1; ++r)
    {
        for (c = 0; c < numCols; ++c)
        {
            mModule->SetConstantR(r, c, bottomConstant);
            diff = mModule->GetPosition(r, c) -
                mModule->GetPosition(r + 1, c);
            mModule->SetLengthR(r, c, Length(diff));
        }
    }
}

void ClothWindow::CreateCloth()
{
    BasisFunctionInput<float> input[2];
    input[0].numControls = mModule->GetNumRows();
    input[1].numControls = mModule->GetNumCols();
    for (int dim = 0; dim < 2; ++dim)
    {
        input[dim].degree = 2;
        input[dim].uniform = true;
        input[dim].periodic = false;
        input[dim].numUniqueKnots =
            input[dim].numControls - input[dim].degree + 1;
        input[dim].uniqueKnots =
            new UniqueKnot<float>[input[dim].numUniqueKnots];
        input[dim].uniqueKnots[0].t = 0.0f;
        input[dim].uniqueKnots[0].multiplicity = input[dim].degree + 1;
        for (int i = 1; i <= input[dim].numUniqueKnots - 2; ++i)
        {
            input[dim].uniqueKnots[i].t =
                i / (input[dim].numUniqueKnots - 1.0f);
            input[dim].uniqueKnots[i].multiplicity = 1;
        }
        input[dim].uniqueKnots[input[dim].numUniqueKnots - 1].t = 1.0f;
        input[dim].uniqueKnots[input[dim].numUniqueKnots - 1].multiplicity =
            input[dim].degree + 1;
    }

    mSpline = std::make_shared<BSplineSurface<3, float>>(input,
        &mModule->GetPosition(0, 0));

    for (int dim = 0; dim < 2; ++dim)
    {
        delete[] input[dim].uniqueKnots;
    }

    mSurface.reset(new RectangleSurface<float>(mSpline));
    mSurface->Set(16, 32);
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

    std::string path = mEnvironment.GetPath("Cloth.png");
    std::shared_ptr<Texture2> texture(WICFileIO::Load(path, true));
    texture->AutogenerateMipmaps();
    std::shared_ptr<Texture2Effect> effect =
        std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::WRAP,
        SamplerState::WRAP);

    mCloth = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mCloth->UpdateModelBound();
    mCloth->localTransform.SetTranslation(-mCloth->modelBound.GetCenter());
    mCameraRig.Subscribe(mCloth->worldTransform,
        effect->GetPVWMatrixConstant());

    mTrackball.Attach(mCloth);
    mTrackball.Update();
}

void ClothWindow::PhysicsTick()
{
    mTime1 = std::chrono::high_resolution_clock::now();
    int64_t delta = std::chrono::duration_cast<std::chrono::milliseconds>(
        mTime1 - mTime0).count();

    mModule->Update(static_cast<float>(delta) / 1000.0f);

    // Update spline surface.  Remember that the spline maintains its own
    // copy of the control points, so this update is necessary.
    for (int r = 0; r < mModule->GetNumRows(); ++r)
    {
        for (int c = 0; c < mModule->GetNumCols(); ++c)
        {
            mSpline->SetControl(r, c, mModule->GetPosition(r, c));
        }
    }

    // Update the GPU copy of the vertices.
    std::shared_ptr<VertexBuffer> vbuffer = mCloth->GetVertexBuffer();
    Vertex* vertices = vbuffer->Get<Vertex>();
    mSurface->GetPositions(&vertices[0].position, sizeof(Vertex));
    mEngine->Update(vbuffer);
    mCameraRig.UpdatePVWMatrices();
}

void ClothWindow::GraphicsTick()
{
    mEngine->ClearBuffers();
    mEngine->Draw(mCloth);
    mEngine->DisplayColorBuffer(0);
}

