// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "StructuredBuffersWindow.h"


StructuredBuffersWindow::StructuredBuffersWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment() || !CreateScene())
    {
        parameters.created = false;
        return;
    }

    InitializeCamera();
}

void StructuredBuffersWindow::OnIdle()
{
    mTimer.Measure();

    mCameraRig.Move();

    memset(mDrawnPixels->GetData(), 0, mDrawnPixels->GetNumBytes());
    mEngine->CopyCpuToGpu(mDrawnPixels);

    mEngine->ClearBuffers();
    mEngine->Draw(mSquare);

    mEngine->CopyGpuToCpu(mDrawnPixels);
    Vector4<float>* src = mDrawnPixels->Get<Vector4<float>>();
    unsigned int* trg = mDrawnPixelsTexture->Get<unsigned int>();
    for (int i = 0; i < mXSize*mYSize; ++i)
    {
        unsigned int r = static_cast<unsigned char>(255.0f*src[i][0]);
        unsigned int g = static_cast<unsigned char>(255.0f*src[i][1]);
        unsigned int b = static_cast<unsigned char>(255.0f*src[i][2]);
        trg[i] = r | (g << 8) | (b << 16) | (0xFF << 24);
    }
    WICFileIO::SaveToPNG("DrawnPixels.png", mDrawnPixelsTexture.get());

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool StructuredBuffersWindow::SetEnvironment()
{
    std::string path = mEnvironment.GetVariable("GTE_PATH");
    if (path == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    mEnvironment.Insert(path + "/Samples/Graphics/StructuredBuffers/Shaders/");

    if (mEnvironment.GetPath("StructuredBuffers.hlsl") == "")
    {
        LogError("Cannot find file StructuredBuffers.hlsl.");
        return false;
    }

    if (mEnvironment.GetPath("StoneWall.png") == "")
    {
        LogError("Cannot find file StoneWall.png.");
        return false;
    }

    return true;
}

bool StructuredBuffersWindow::CreateScene()
{
    // Create the shaders and associated resources.
    std::string filename = mEnvironment.GetPath("StructuredBuffers.hlsl");
    mProgramFactory.defines.Set("WINDOW_WIDTH", mXSize);
    std::shared_ptr<VisualProgram> program =
        mProgramFactory.CreateFromFiles(filename, filename, "");
    if (!program)
    {
        return false;
    }
    mProgramFactory.defines.Clear();

    std::shared_ptr<ConstantBuffer> cbuffer(new ConstantBuffer(
        sizeof(Matrix4x4<float>), true));
    program->GetVShader()->Set("PVWMatrix", cbuffer);

    // Create the pixel shader and associated resources.
    std::shared_ptr<PixelShader> pshader = program->GetPShader();
    std::string path = mEnvironment.GetPath("StoneWall.png");
    std::shared_ptr<Texture2> baseTexture(WICFileIO::Load(path, false));
    pshader->Set("baseTexture", baseTexture);

    std::shared_ptr<SamplerState> baseSampler(new SamplerState());
    baseSampler->filter = SamplerState::MIN_L_MAG_L_MIP_P;
    baseSampler->mode[0] = SamplerState::CLAMP;
    baseSampler->mode[1] = SamplerState::CLAMP;
    pshader->Set("baseSampler", baseSampler);

    mDrawnPixels = std::make_shared<StructuredBuffer>(mXSize*mYSize,
        sizeof(Vector4<float>));
    mDrawnPixels->SetUsage(Resource::SHADER_OUTPUT);
    mDrawnPixels->SetCopyType(Resource::COPY_BIDIRECTIONAL);
    memset(mDrawnPixels->GetData(), 0, mDrawnPixels->GetNumBytes());
    pshader->Set("drawnPixels", mDrawnPixels);

    // Create the visual effect for the square.
    std::shared_ptr<VisualEffect> effect =
        std::make_shared<VisualEffect>(program);

    // Create a vertex buffer for a single triangle.  The PNG is stored in
    // left-handed coordinates.  The texture coordinates are chosen to reflect
    // the texture in the y-direction.
    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
    std::shared_ptr<VertexBuffer> vbuffer(new VertexBuffer(vformat, 4));
    Vertex* vertex = vbuffer->Get<Vertex>();
    vertex[0].position = { 0.0f, 0.0f, 0.0f };
    vertex[0].tcoord = { 0.0f, 1.0f };
    vertex[1].position = { 1.0f, 0.0f, 0.0f };
    vertex[1].tcoord = { 1.0f, 1.0f };
    vertex[2].position = { 0.0f, 1.0f, 0.0f };
    vertex[2].tcoord = { 0.0f, 0.0f };
    vertex[3].position = { 1.0f, 1.0f, 0.0f };
    vertex[3].tcoord = { 1.0f, 0.0f };

    // Create an indexless buffer for a triangle mesh with two triangles.
    std::shared_ptr<IndexBuffer> ibuffer(new IndexBuffer(IP_TRISTRIP, 2));

    // Create the geometric object for drawing.  Translate it so that its
    // center of mass is at the origin.  This supports virtual trackball
    // motion about the object "center".
    mSquare = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mSquare->localTransform.SetTranslation(-0.5f, -0.5f, 0.0f);

    // Enable automatic updates of pvw-matrices and w-matrices.
    mCameraRig.Subscribe(mSquare->worldTransform, cbuffer);

    // The structured buffer is written in the pixel shader.  This texture
    // will receive a copy of it so that we can write the results to disk
    // as a PNG file.
    mDrawnPixelsTexture = std::make_unique<Texture2>(DF_R8G8B8A8_UNORM,
        mXSize, mYSize);

    mTrackball.Attach(mSquare);
    mTrackball.Update();
    return true;
}

void StructuredBuffersWindow::InitializeCamera()
{
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 0.1f, 100.0f);
    Vector4<float> camPosition{ 0.0f, 0.0f, 1.25f, 1.0f };
    Vector4<float> camDVector{ 0.0f, 0.0f, -1.0f, 0.0f };
    Vector4<float> camUVector{ 0.0f, 1.0f, 0.0f, 0.0f };
    Vector4<float> camRVector = Cross(camDVector, camUVector);
    mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

    mTranslationSpeed = 0.001f;
    mRotationSpeed = 0.001f;
    mTranslationSpeedFactor = 2.0f;
    mRotationSpeedFactor = 2.0f;
    mCameraRig.SetTranslationSpeed(mTranslationSpeed);
    mCameraRig.SetRotationSpeed(mRotationSpeed);
    mCameraRig.ComputeWorldAxes();
    mCameraRig.UpdatePVWMatrices();
}

