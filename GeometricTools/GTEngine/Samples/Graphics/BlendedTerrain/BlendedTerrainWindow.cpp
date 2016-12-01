// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "BlendedTerrainWindow.h"


BlendedTerrainWindow::BlendedTerrainWindow(Parameters& parameters)
    :
    Window3(parameters),
    mFlowDelta(0.00002f),
    mPowerDelta(1.125f),
    mZAngle(0.0f),
    mZDeltaAngle(0.00002f)
{
    if (!SetEnvironment() || !CreateTerrain())
    {
        parameters.created = false;
        return;
    }

    CreateSkyDome();
    InitializeCamera();

    mWireState = std::make_shared<RasterizerState>();
    mWireState->fillMode = RasterizerState::FILL_WIREFRAME;
}

void BlendedTerrainWindow::OnIdle()
{
    mTimer.Measure();

    mCameraRig.Move();
    Update();

    mEngine->ClearBuffers();
    mEngine->Draw(mTerrain);
    mEngine->Draw(mSkyDome);
    mEngine->Draw(8, GetYSize() - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool BlendedTerrainWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mEngine->GetRasterizerState() == mWireState)
        {
            mEngine->SetDefaultRasterizerState();
        }
        else
        {
            mEngine->SetRasterizerState(mWireState);
        }
        return true;

    case 'p':
    case 'P':
        mTerrainEffect->SetPowerFactor(mTerrainEffect->GetPowerFactor()*mPowerDelta);
        mEngine->Update(mTerrainEffect->GetPowerFactorConstant());
        return true;

    case 'm':
    case 'M':
        mTerrainEffect->SetPowerFactor(mTerrainEffect->GetPowerFactor()/mPowerDelta);
        mEngine->Update(mTerrainEffect->GetPowerFactorConstant());
        return true;
    }

    return Window::OnCharPress(key, x, y);
}

bool BlendedTerrainWindow::SetEnvironment()
{
    std::string path = mEnvironment.GetVariable("GTE_PATH");
    if (path == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return false;
    }
    mEnvironment.Insert(path + "/Samples/Graphics/BlendedTerrain/Shaders/");
    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("BTHeightField.png") == "")
    {
        LogError("Cannot find file BTHeightField.png.");
        return false;
    }

    if (mEnvironment.GetPath("BTGrass.png") == "")
    {
        LogError("Cannot find file BTGrass.png.");
        return false;
    }

    if (mEnvironment.GetPath("BTStone.png") == "")
    {
        LogError("Cannot find file BTStone.png.");
        return false;
    }

    if (mEnvironment.GetPath("BTCloud.png") == "")
    {
        LogError("Cannot find file BTCloud.png.");
        return false;
    }

    if (mEnvironment.GetPath("SkyDome.png") == "")
    {
        LogError("Cannot find file SkyDome.png.");
        return false;
    }

    if (mEnvironment.GetPath("SkyDome.txt") == "")
    {
        LogError("Cannot find file SkyDome.txt.");
        return false;
    }

    return true;
}

bool BlendedTerrainWindow::CreateTerrain()
{
    // Load the height field for vertex displacement.
    std::string heightFile = mEnvironment.GetPath("BTHeightField.png");

    // Create the visual effect.
    bool created = false;
    mTerrainEffect = std::make_shared<BlendedTerrainEffect>(mProgramFactory,
        mEnvironment, created);
    if (!created)
    {
        LogError("Failed to create the terrain effect.");
        return false;
    }

    // Create the vertex buffer for terrain.
    unsigned int const numSamples0 = 64, numSamples1 = 64;
    unsigned int const numVertices = numSamples0*numSamples1;

    struct TerrainVertex
    {
        Vector3<float> position;
        Vector2<float> tcoord0;
        float tcoord1;
        Vector2<float> tcoord2;
    };
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32_FLOAT, 1);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 2);
    std::shared_ptr<VertexBuffer> vbuffer =
        std::make_shared<VertexBuffer>(vformat, numVertices);

    // Generate the geometry for a flat height field.
    TerrainVertex* vertex = vbuffer->Get<TerrainVertex>();
    float const extent0 = 8.0f, extent1 = 8.0f;
    float const inv0 = 1.0f / (static_cast<float>(numSamples0) - 1.0f);
    float const inv1 = 1.0f / (static_cast<float>(numSamples1) - 1.0f);
    Vector3<float> position;
    Vector2<float> tcoord;
    unsigned int i, i0, i1;
    for (i1 = 0, i = 0; i1 < numSamples1; ++i1)
    {
        tcoord[1] = i1 * inv1;
        position[1] = (2.0f * tcoord[1] - 1.0f) * extent1;
        for (i0 = 0; i0 < numSamples0; ++i0, ++i)
        {
            tcoord[0] = i0 * inv0;
            position[0] = (2.0f * tcoord[0] - 1.0f) * extent0;
            vertex[i].position = position;
            vertex[i].tcoord0 = tcoord;
            vertex[i].tcoord1 = 0.0f;
            vertex[i].tcoord2 = tcoord;
        }
    }

    // Use a Mersenne twister engine for random numbers.
    std::mt19937 mte;
    std::uniform_real_distribution<float> symrnd(-1.0f, 1.0f);

    // Set the heights based on a precomputed height field.  The image is
    // known to be 64x64, which matches numSamples0 and numSamples1.  It is
    // also gray scale, so we use only the red channel.
    Texture2* texture = WICFileIO::Load(heightFile, false);
    unsigned char* image = texture->Get<unsigned char>();
    for (i = 0; i < numVertices; i++)
    {
        float height = static_cast<float>(image[4 * i])/255.0f;
        float perturb = 0.05f * symrnd(mte);
        vertex[i].position[2] = 3.0f * height + perturb;
        vertex[i].tcoord0 *= 8.0f;
        vertex[i].tcoord1 = height;
    }
    delete texture;

    // Generate the index array for a regular grid of squares, each square a
    // pair of triangles.
    unsigned int const numTriangles = 2 * (numSamples0 - 1) * (numSamples1 - 1);
    std::shared_ptr<IndexBuffer> ibuffer =
        std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(unsigned int));
    unsigned int* indices = ibuffer->Get<unsigned int>();
    for (i1 = 0, i = 0; i1 < numSamples1 - 1; ++i1)
    {
        for (i0 = 0; i0 < numSamples0 - 1; ++i0)
        {
            int v0 = i0 + numSamples0*i1;
            int v1 = v0 + 1;
            int v2 = v1 + numSamples0;
            int v3 = v0 + numSamples0;
            *indices++ = v0;
            *indices++ = v1;
            *indices++ = v2;
            *indices++ = v0;
            *indices++ = v2;
            *indices++ = v3;
        }
    }

    // Create the visual object.
    mTerrain = std::make_shared<Visual>(vbuffer, ibuffer, mTerrainEffect);
    mCameraRig.Subscribe(mTerrain->worldTransform, mTerrainEffect->GetPVWMatrixConstant());
    mTrackball.Attach(mTerrain);
    return true;
}

void BlendedTerrainWindow::CreateSkyDome()
{
    // Load the vertices and indices from file for the sky dome trimesh.
    std::string name = mEnvironment.GetPath("SkyDome.txt");
    std::ifstream inFile(name.c_str());

    unsigned int numVertices, numIndices, i;
    inFile >> numVertices;
    inFile >> numIndices;

    struct SkyDomeVertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    std::shared_ptr<VertexBuffer> vbuffer =
        std::make_shared<VertexBuffer>(vformat, numVertices);
    SkyDomeVertex* vertex = vbuffer->Get<SkyDomeVertex>();
    for (i = 0; i < numVertices; ++i)
    {
        Vector3<float> position;
        Vector2<float> tcoord;
        inFile >> vertex[i].position[0];
        inFile >> vertex[i].position[1];
        inFile >> vertex[i].position[2];
        inFile >> vertex[i].tcoord[0];
        inFile >> vertex[i].tcoord[1];
    }

    int const numTriangles = numIndices/3;
    std::shared_ptr<IndexBuffer> ibuffer =
        std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(unsigned int));
    int* indices = ibuffer->Get<int>();
    for (i = 0; i < numIndices; ++i, ++indices)
    {
        inFile >> *indices;
    }

    inFile.close();

    // Load the sky texture.
    name = mEnvironment.GetPath("SkyDome.png");
    std::shared_ptr<Texture2> sky(WICFileIO::Load(name, true));
    sky->AutogenerateMipmaps();

    // Create the visual effect.
    mSkyDomeEffect = std::make_shared<Texture2Effect>(mProgramFactory, sky,
        SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::WRAP,
        SamplerState::WRAP);

    // Create the visual object.
    mSkyDome = std::make_shared<Visual>(vbuffer, ibuffer, mSkyDomeEffect);

    // The sky dome needs to be translated and scaled for this sample.
    mSkyDome->localTransform.SetUniformScale(7.9f);
    mSkyDome->localTransform.SetTranslation(0.0f, 0.0f, -0.1f);
    mSkyDome->Update();
    mCameraRig.Subscribe(mSkyDome->worldTransform, mSkyDomeEffect->GetPVWMatrixConstant());
    mTrackball.Attach(mSkyDome);
}

void BlendedTerrainWindow::InitializeCamera()
{
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 0.01f, 100.0f);
    Vector4<float> camPosition{ 0.0f, -7.0f, 1.5f, 1.0f };
    Vector4<float> camDVector{ 0.0f, 1.0f, 0.0f, 0.0f };
    Vector4<float> camUVector{ 0.0f, 0.0f, 1.0f, 0.0f };
    Vector4<float> camRVector = Cross(camDVector, camUVector);
    mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

    mTranslationSpeed = 0.005f;
    mRotationSpeed = 0.002f;
    mTranslationSpeedFactor = 2.0f;
    mRotationSpeedFactor = 2.0f;
    mCameraRig.SetTranslationSpeed(mTranslationSpeed);
    mCameraRig.SetRotationSpeed(mRotationSpeed);
    mCameraRig.ComputeWorldAxes();
    mCameraRig.UpdatePVWMatrices();
}

void BlendedTerrainWindow::Update()
{
    // Animate the cloud layer.
    Vector2<float> flowDirection = mTerrainEffect->GetFlowDirection();
    flowDirection[0] += mFlowDelta;
    if (0.0f > flowDirection[0])
    {
        flowDirection[0] += 1.0f;
    }
    else if (1.0f < flowDirection[0])
    {
        flowDirection[0] -= 1.0f;
    }
    mTerrainEffect->SetFlowDirection(flowDirection);
    mEngine->Update(mTerrainEffect->GetFlowDirectionConstant());

    // Rotate the sky dome.
    mZAngle -= mZDeltaAngle;
    if (mZAngle < (float)-GTE_C_TWO_PI)
    {
        mZAngle += (float)GTE_C_TWO_PI;
    }
    mSkyDome->localTransform.SetRotation(
        AxisAngle<4, float>({ 0.0f, 0.0f, 1.0f, 0.0f }, -mZAngle));
    mSkyDome->Update();
    mCameraRig.UpdatePVWMatrices();
}

