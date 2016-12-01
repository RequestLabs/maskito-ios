// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "LightTextureWindow.h"

LightTextureWindow::LightTextureWindow(Parameters& parameters)
    :
    Window3(parameters),
    mUseDirectional(true)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({ 0.525f, 0.741f, 0.831f, 1.0f });
    InitializeCamera();
    CreateScene();
}

void LightTextureWindow::OnIdle()
{
    mTimer.Measure();

    mCameraRig.Move();
    UpdateConstants();

    mEngine->ClearBuffers();
    mEngine->Draw(mTerrain);

    Vector4<float> textColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    mEngine->Draw(8, mYSize - 24, textColor, (mUseDirectional ? "Directional" : "Point"));
    mEngine->Draw(8, mYSize - 8, textColor, mTimer.GetFPS());

    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool LightTextureWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 's':
    case 'S':
        if (mUseDirectional)
        {
            mCameraRig.Unsubscribe(mTerrain->worldTransform);
            mTerrain->SetEffect(mPLTEffect);
            mCameraRig.Subscribe(mTerrain->worldTransform, mPLTEffect->GetPVWMatrixConstant());
            mCameraRig.UpdatePVWMatrices();
        }
        else
        {
            mCameraRig.Unsubscribe(mTerrain->worldTransform);
            mTerrain->SetEffect(mDLTEffect);
            mCameraRig.Subscribe(mTerrain->worldTransform, mDLTEffect->GetPVWMatrixConstant());
            mCameraRig.UpdatePVWMatrices();
        }
        mUseDirectional = !mUseDirectional;
        return true;
    }
    return Window3::OnCharPress(key, x, y);
}

bool LightTextureWindow::SetEnvironment()
{
    std::string path = mEnvironment.GetVariable("GTE_PATH");
    if (path == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return false;
    }
    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("BTHeightField.png") == "")
    {
        LogError("Cannot find file BTHeightField.png.");
        return false;
    }

    if (mEnvironment.GetPath("BTStone.png") == "")
    {
        LogError("Cannot find file BTStone.png.");
        return false;
    }

    return true;
}

void LightTextureWindow::InitializeCamera()
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
}

void LightTextureWindow::CreateScene()
{
    mTrackball.Set(mXSize, mYSize, mCamera);

    // Create the visual effect.  The world up-direction is (0,0,1).  Choose
    // the light to point down.
    std::shared_ptr<Material> material = std::make_shared<Material>();
    material->emissive = { 0.0f, 0.0f, 0.0f, 1.0f };
    material->ambient = { 0.5f, 0.5f, 0.5f, 1.0f };
    material->diffuse = { 0.5f, 0.5f, 0.5f, 1.0f };
    material->specular = { 1.0f, 1.0f, 1.0f, 75.0f };

    std::shared_ptr<Lighting> lighting = std::make_shared<Lighting>();
    lighting->ambient = mEngine->GetClearColor();
    lighting->attenuation = { 1.0f, 0.0f, 0.0f, 1.0f };

    std::shared_ptr<LightCameraGeometry> geometry = std::make_shared<LightCameraGeometry>();
    mLightWorldPosition = { 0.0f, 0.0f, 8.0f, 1.0f };
    mLightWorldDirection = { 0.0f, 0.0f, -1.0f, 0.0f };

    std::string stoneFile = mEnvironment.GetPath("BTStone.png");
    std::shared_ptr<Texture2> stoneTexture(WICFileIO::Load(stoneFile, true));
    stoneTexture->AutogenerateMipmaps();

    mDLTEffect = std::make_shared<DirectionalLightTextureEffect>(mProgramFactory,
        mUpdater, material, lighting, geometry, stoneTexture,
        SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::CLAMP, SamplerState::CLAMP);

    mPLTEffect = std::make_shared<PointLightTextureEffect>(mProgramFactory,
        mUpdater, material, lighting, geometry, stoneTexture,
        SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::CLAMP, SamplerState::CLAMP);

    // Create the height field for terrain using heights from a gray-scale
    // bitmap image.
    struct Vertex
    {
        Vector3<float> position, normal;
        Vector2<float> tcoord;
    };
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_NORMAL, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    std::string heightFile = mEnvironment.GetPath("BTHeightField.png");
    Texture2* heightTexture = WICFileIO::Load(heightFile, false);
    MeshFactory mf;  // Fills in Vertex.position, Vertex.tcoord.
    mf.SetVertexFormat(vformat);
    mTerrain = mf.CreateRectangle(
        heightTexture->GetWidth(), heightTexture->GetHeight(), 8.0f, 8.0f);
    mTrackball.Attach(mTerrain);

    // The mesh factory creates a flat height field.  Use the height-field
    // image to generate the heights and use a random number generator to
    // perturb them, just to add some noise.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(-1.0f, 1.0f);
    std::shared_ptr<VertexBuffer> vbuffer = mTerrain->GetVertexBuffer();
    unsigned int numVertices = vbuffer->GetNumElements();
    Vertex* vertex = vbuffer->Get<Vertex>();
    unsigned char* heights = heightTexture->Get<unsigned char>();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        float height = static_cast<float>(heights[4 * i]) / 255.0f;
        float perturb = 0.05f * rnd(mte);
        vertex[i].position[2] = 3.0f * height + perturb;
    }
    delete heightTexture;

    mTerrain->SetEffect(mDLTEffect);
    mTerrain->UpdateModelNormals();  // Fill in Vertex.normal.
    mCameraRig.Subscribe(mTerrain->worldTransform, mDLTEffect->GetPVWMatrixConstant());
}

void LightTextureWindow::UpdateConstants()
{
    Matrix4x4<float> invWMatrix = mTerrain->worldTransform.GetHInverse();
    Vector4<float> cameraWorldPosition = mCamera->GetPosition();
    std::shared_ptr<LightCameraGeometry> const& geometry = mDLTEffect->GetGeometry();
#if defined(GTE_USE_MAT_VEC)
    geometry->cameraModelPosition = invWMatrix * cameraWorldPosition;
    if (mUseDirectional)
    {
        geometry->lightModelDirection = invWMatrix * mLightWorldDirection;
        mDLTEffect->UpdateGeometryConstant();
    }
    else
    {
        geometry->lightModelPosition = invWMatrix * mLightWorldPosition;
        mPLTEffect->UpdateGeometryConstant();
    }
#else
    geometry->cameraModelPosition = cameraWorldPosition * invWMatrix;
    if (mUseDirectional)
    {
        geometry->lightModelDirection = mLightWorldDirection * invWMatrix;
        mDLTEffect->UpdateGeometryConstant();
    }
    else
    {
        geometry->lightModelPosition = mLightWorldPosition * invWMatrix;
        mPLTEffect->UpdateGeometryConstant();
    }
#endif
    mCameraRig.UpdatePVWMatrices();
}

