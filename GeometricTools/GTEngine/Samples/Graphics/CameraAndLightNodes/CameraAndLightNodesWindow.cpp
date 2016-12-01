// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "CameraAndLightNodesWindow.h"

CameraAndLightNodesWindow::CameraAndLightNodesWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::BM_SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::BM_INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::BM_SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::BM_INV_SRC_ALPHA;

    mNoDepthStencilState = std::make_shared<DepthStencilState>();
    mNoDepthStencilState->depthEnable = false;
    mNoDepthStencilState->stencilEnable = false;

    mWireState = std::make_shared<RasterizerState>();
    mWireState->fillMode = RasterizerState::FILL_WIREFRAME;

    std::string path = mEnvironment.GetPath("RedSky.png");
    std::shared_ptr<Texture2> skyTexture(WICFileIO::Load(path, false));
    mOverlay = std::make_shared<OverlayEffect>(mProgramFactory, mXSize, mYSize, mXSize, mYSize,
        SamplerState::MIN_P_MAG_P_MIP_P, SamplerState::CLAMP, SamplerState::CLAMP, true);
    mOverlay->SetTexture(skyTexture);

    // The order of these calls is important.  The camera and camera node must
    // be created first so that the camera node has the correct transforms
    // initially.  The camera node is then attached to the scene and the update
    // works correctly.
    InitializeCameraNode();
    CreateScene();

    mScene->Update();
    mCameraNodeRig.UpdatePVWMatrices();
}

void CameraAndLightNodesWindow::OnIdle()
{
    mTimer.Measure();

    mCameraNodeRig.Move();

    mEngine->ClearBuffers();

    // Draw the red-sky background.
    mEngine->SetDepthStencilState(mNoDepthStencilState);
    mEngine->Draw(mOverlay);
    mEngine->SetDefaultDepthStencilState();

    // Draw the ground and light disks.
    mEngine->Draw(mGround);
    mEngine->SetBlendState(mBlendState);
    mEngine->Draw(mLightTarget[0]);
    mEngine->Draw(mLightTarget[1]);
    mEngine->SetDefaultBlendState();

    mEngine->Draw(8, mYSize - 8, { 1.0f, 1.0f, 1.0f, 1.0f }, mTimer.GetFPS());

    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool CameraAndLightNodesWindow::OnCharPress(unsigned char key, int x, int y)
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

    case '+':  // increase light intensity
    case '=':
        for (int i = 0; i < 2; ++i)
        {
            auto lighting = mEffect[i]->GetLighting();
            lighting->attenuation[3] += 0.1f;
            mEffect[i]->UpdateLightingConstant();
        }
        return true;

    case '-':  // decrease light intensity
    case '_':
        if (mEffect[0]->GetLighting()->attenuation[3] >= 0.1f)
        {
            for (int i = 0; i < 2; ++i)
            {
                auto lighting = mEffect[i]->GetLighting();
                lighting->attenuation[3] -= 0.1f;
                mEffect[i]->UpdateLightingConstant();
            }
        }
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

bool CameraAndLightNodesWindow::OnKeyDown(int key, int, int)
{
    return mCameraNodeRig.PushMotion(key);
}

bool CameraAndLightNodesWindow::OnKeyUp(int key, int, int)
{
    return mCameraNodeRig.PopMotion(key);
}

bool CameraAndLightNodesWindow::SetEnvironment()
{
    std::string path = mEnvironment.GetVariable("GTE_PATH");
    if (path == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return false;
    }
    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("Gravel.png") == "")
    {
        LogError("Cannot find file Gravel.png.");
        return false;
    }

    if (mEnvironment.GetPath("RedSky.png") == "")
    {
        LogError("Cannot find file RedSky.png.");
        return false;
    }

    return true;
}

void CameraAndLightNodesWindow::InitializeCameraNode()
{
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 1.0f, 1000.0f);
    Vector4<float> camPosition{ 0.0f, -100.0f, 5.0f, 1.0f };
    Vector4<float> camDVector{ 0.0f, 1.0f, 0.0f, 0.0f };
    Vector4<float> camUVector{ 0.0f, 0.0f, 1.0f, 0.0f };
    Vector4<float> camRVector = Cross(camDVector, camUVector);
    mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

    mCameraNode = std::make_shared<ViewVolumeNode>(mCamera);
    mCameraNode->SetOnUpdate([this](ViewVolumeNode* cameraNode)
    {
        auto const& invWMatrix = mGround->worldTransform.GetHInverse();
        auto const& cameraWorldPosition = cameraNode->GetViewVolume()->GetPosition();
#if defined(GTE_USE_MAT_VEC)
        mCameraModelPosition = invWMatrix *cameraWorldPosition;
#else
        mCameraModelPosition = cameraWorldPosition * invWMatrix;
#endif
    });
    mCameraNodeRig.SetCameraNode(mCameraNode);

    mTranslationSpeed = 0.01f;
    mRotationSpeed = 0.001f;
    mTranslationSpeedFactor = 2.0f;
    mRotationSpeedFactor = 2.0f;

    mCameraNodeRig.Set(mCamera, mTranslationSpeed, mRotationSpeed, mUpdater);
    mCameraNodeRig.RegisterMoveForward(VK_UP);
    mCameraNodeRig.RegisterMoveBackward(VK_DOWN);
    mCameraNodeRig.RegisterTurnRight(VK_RIGHT);
    mCameraNodeRig.RegisterTurnLeft(VK_LEFT);
}

void CameraAndLightNodesWindow::CreateScene()
{
    // scene -+--> groundPoly
    //        |
    //        +--> cameraNode --+--> lightFixture0 +--> lightNode0
    //                          |                  |
    //                          |                  +--> lightTarget0
    //                          |
    //                          +--> lightFixture1 +--> lightNode1
    //                                             |
    //                                             +--> lightTarget0

    mScene = std::make_shared<Node>();
    mScene->AttachChild(CreateGround());
    mScene->AttachChild(mCameraNode);

    AxisAngle<4, float> aa(Vector4<float>::Unit(0), (float)-GTE_C_HALF_PI);
    std::shared_ptr<Node> lightFixture0 = CreateLightFixture(0);
    lightFixture0->localTransform.SetTranslation(25.0f, -5.75f, 6.0f);
    lightFixture0->localTransform.SetRotation(aa);
    mCameraNode->AttachChild(lightFixture0);

    std::shared_ptr<Node> lightFixture1 = CreateLightFixture(1);
    lightFixture1->localTransform.SetTranslation(25.0f, -5.75f, -6.0f);
    lightFixture1->localTransform.SetRotation(aa);
    mCameraNode->AttachChild(lightFixture1);
}

std::shared_ptr<Visual> CameraAndLightNodesWindow::CreateGround()
{
    struct Vertex
    {
        Vector3<float> position, normal;
        Vector2<float> tcoord;
    };

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_NORMAL, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    std::shared_ptr<VertexBuffer> vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    Vertex* vertex = vbuffer->Get<Vertex>();
    vertex[0].position = Vector3<float>{ -100.0f, -100.0f, 0.0f };
    vertex[1].position = Vector3<float>{ +100.0f, -100.0f, 0.0f };
    vertex[2].position = Vector3<float>{ +100.0f, +100.0f, 0.0f };
    vertex[3].position = Vector3<float>{ -100.0f, +100.0f, 0.0f };
    vertex[0].normal = Vector3<float>::Unit(2);
    vertex[1].normal = Vector3<float>::Unit(2);
    vertex[2].normal = Vector3<float>::Unit(2);
    vertex[3].normal = Vector3<float>::Unit(2);
    vertex[0].tcoord = Vector2<float>{ 0.0f, 0.0f };
    vertex[1].tcoord = Vector2<float>{ 8.0f, 0.0f };
    vertex[2].tcoord = Vector2<float>{ 8.0f, 8.0f };
    vertex[3].tcoord = Vector2<float>{ 0.0f, 8.0f };

    std::shared_ptr<IndexBuffer> ibuffer = std::make_shared<IndexBuffer>(
        IP_TRIMESH, 2, sizeof(unsigned int));
    ibuffer->SetTriangle(0, 0, 1, 2);
    ibuffer->SetTriangle(1, 0, 2, 3);

    std::string path = mEnvironment.GetPath("Gravel.png");
    std::shared_ptr<Texture2> gravelTexture(WICFileIO::Load(path, true));
    gravelTexture->AutogenerateMipmaps();

    // Darken the gravel.
    unsigned int* texels = gravelTexture->Get<unsigned int>();
    for (unsigned int i = 0; i < gravelTexture->GetNumElements(); ++i)
    {
        unsigned int r = (unsigned int)(0.2f * (texels[i] & 0x000000FF));
        unsigned int g = (unsigned int)(0.2f * ((texels[i] & 0x0000FF00) >> 8));
        unsigned int b = (unsigned int)(0.2f * ((texels[i] & 0x00FF0000) >> 16));
        texels[i] = r | (g << 8) | (b << 16) | 0xFF000000;
    }

    std::shared_ptr<Texture2Effect> effect = std::make_shared<Texture2Effect>(
        mProgramFactory, gravelTexture, SamplerState::MIN_L_MAG_L_MIP_L,
        SamplerState::WRAP, SamplerState::WRAP);

    mGround = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mCameraNodeRig.Subscribe(mGround->worldTransform, effect->GetPVWMatrixConstant());
    return mGround;
}

std::shared_ptr<Node> CameraAndLightNodesWindow::CreateLightFixture(int i)
{
    std::shared_ptr<Node> lightFixture = std::make_shared<Node>();

    // A point light illuminates the target.  Create a semitransparent
    // material for the patch.
    std::shared_ptr<Material> material = std::make_shared<Material>();
    std::shared_ptr<LightCameraGeometry> geometry = std::make_shared<LightCameraGeometry>();
    std::shared_ptr<Light> light = std::make_shared<Light>();
    light->lighting = std::make_shared<Lighting>();

    material->emissive = { 0.0f, 0.0f, 0.0f, 1.0f };
    material->ambient = { 0.5f, 0.5f, 0.5f, 1.0f };
    material->diffuse = { 1.0f, 0.85f, 0.75f, 0.5f };
    material->specular = { 0.8f, 0.8f, 0.8f, 1.0f };
    light->lighting->ambient = { 1.0f, 1.0f, 0.5f, 1.0f };
    light->lighting->diffuse = { 1.0f, 1.0f, 0.5f, 1.0f };
    light->lighting->specular = { 1.0f, 1.0f, 0.5f, 1.0f };
    light->SetPosition({ 0.0f, 0.0f, 0.0f, 1.0f });

    // Create the target itself.  The incoming light is a point light, so use
    // an effect that combines this light with the specified material.
    mLightTarget[i] = CreateLightTarget();

    mEffect[i] = std::make_shared<PointLightEffect>(
        mProgramFactory, mUpdater, 0, material, light->lighting, geometry);
    mLightTarget[i]->SetEffect(mEffect[i]);

    mCameraNodeRig.Subscribe(mLightTarget[i]->worldTransform, mEffect[i]->GetPVWMatrixConstant());

    // Encapsulate the light in a light node.  Rotate the light node so the
    // light points downward.
    mLightNode[i] = std::make_shared<ViewVolumeNode>(light);
    mLightNode[i]->SetOnUpdate([this, i](ViewVolumeNode*)
    {
        // The camera model position must be updated for the light targets to
        // move.  The light model position is not updated because the point
        // lights must move with their corresponding light targets.
        auto geometry = mEffect[i]->GetGeometry();
        geometry->cameraModelPosition = mCameraModelPosition;
        mEffect[i]->UpdateGeometryConstant();
    });

    lightFixture->AttachChild(mLightNode[i]);
    lightFixture->AttachChild(mLightTarget[i]);
    return lightFixture;
}

std::shared_ptr<Visual> CameraAndLightNodesWindow::CreateLightTarget()
{
    // Create a parabolic rectangle patch that is illuminated by the light.
    // To hide the artifacts of vertex normal lighting on a grid, the patch
    // is slightly bent so that the intersection with a plane is nearly
    // circular.  The patch is translated slightly below the plane of the
    // ground to hide the corners and the jaggies.
    struct Vertex
    {
        Vector3<float> position, normal;
    };

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_NORMAL, DF_R32G32B32_FLOAT, 0);

    // Create a flat surface.
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    std::shared_ptr<Visual> mesh = mf.CreateRectangle(64, 64, 8.0f, 8.0f);

    // Adjust the heights to form a paraboloid.
    std::shared_ptr<VertexBuffer> vbuffer = mesh->GetVertexBuffer();
    unsigned int const numVertices = vbuffer->GetNumActiveElements();
    Vertex* vertex = vbuffer->Get<Vertex>();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        Vector3<float>& pos = vertex[i].position;
        pos[2] = 1.0f - (pos[0] * pos[0] + pos[1] * pos[1]) / 128.0f;
    }
    mesh->UpdateModelNormals();

    return mesh;
}

void CameraAndLightNodesWindow::CameraNodeRig::SetCameraNode(
    std::shared_ptr<ViewVolumeNode> const& cameraNode)
{
    mCameraNode = cameraNode;
}

void CameraAndLightNodesWindow::CameraNodeRig::MoveForward()
{
    Vector4<float> position = mCameraNode->localTransform.GetTranslationW1();
#if defined(GTE_USE_MAT_VEC)
    Vector4<float> direction = mCameraNode->localTransform.GetRotation().GetCol(0);
#else
    Vector4<float> direction = mCameraNode->localTransform.GetRotation().GetRow(0);
#endif
    position += mTranslationSpeed * direction;
    mCameraNode->localTransform.SetTranslation(position);
    mCameraNode->Update();
}

void CameraAndLightNodesWindow::CameraNodeRig::MoveBackward()
{
    Vector4<float> position = mCameraNode->localTransform.GetTranslationW1();
#if defined(GTE_USE_MAT_VEC)
    Vector4<float> direction = mCameraNode->localTransform.GetRotation().GetCol(0);
#else
    Vector4<float> direction = mCameraNode->localTransform.GetRotation().GetRow(0);
#endif
    position -= mTranslationSpeed * mCamera->GetDVector();
    mCameraNode->localTransform.SetTranslation(position);
    mCameraNode->Update();
}

void CameraAndLightNodesWindow::CameraNodeRig::TurnRight()
{
    Matrix4x4<float> rotate = mCameraNode->localTransform.GetRotation();
#if defined(GTE_USE_MAT_VEC)
    Vector4<float> uVector = rotate.GetCol(1);
#else
    Vector4<float> uVector = rotate.GetRow(1);
#endif
    AxisAngle<4, float> aa(uVector, -mRotationSpeed);
    Matrix4x4<float> increment = Rotation<4, float>(aa);
#if defined(GTE_USE_MAT_VEC)
    mCameraNode->localTransform.SetRotation(increment * rotate);
#else
    mCameraNode->localTransform.SetRotation(rotate * increment);
#endif
    mCameraNode->Update();
}

void CameraAndLightNodesWindow::CameraNodeRig::TurnLeft()
{
    Matrix4x4<float> rotate = mCameraNode->localTransform.GetRotation();
#if defined(GTE_USE_MAT_VEC)
    Vector4<float> uVector = rotate.GetCol(1);
#else
    Vector4<float> uVector = rotate.GetRow(1);
#endif
    AxisAngle<4, float> aa(uVector, +mRotationSpeed);
    Matrix4x4<float> increment = Rotation<4, float>(aa);
#if defined(GTE_USE_MAT_VEC)
    mCameraNode->localTransform.SetRotation(increment * rotate);
#else
    mCameraNode->localTransform.SetRotation(rotate * increment);
#endif
    mCameraNode->Update();
}
