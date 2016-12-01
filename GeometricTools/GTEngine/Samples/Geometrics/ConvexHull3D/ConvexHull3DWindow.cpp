// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "ConvexHull3DWindow.h"


ConvexHull3DWindow::ConvexHull3DWindow(Parameters& parameters)
    :
    Window3(parameters),
    mFileQuantity(46),
    mCurrentFile(1)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mEffect = std::make_shared<VertexColorEffect>(mProgramFactory);
    if (!LoadData())
    {
        parameters.created = false;
        return;
    }

    mWireState = std::make_shared<RasterizerState>();
    mWireState->cullMode = RasterizerState::CULL_NONE;
    mWireState->fillMode = RasterizerState::FILL_WIREFRAME;
}

void ConvexHull3DWindow::OnIdle()
{
    mTimer.Measure();

    mCameraRig.Move();

    mEngine->ClearBuffers();

    if (mMesh)
    {
        mEngine->Draw(mMesh);
    }

    Vector4<float> textColor{ 0.0f, 0.0f, 0.0f, 1.0f };
    if (mMessage != "")
    {
        mEngine->Draw(8, 16, textColor, mMessage);
    }

    mEngine->Draw(8, mYSize - 8, textColor, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool ConvexHull3DWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'd':  // Load a new data set.
    case 'D':
        if (++mCurrentFile == mFileQuantity)
        {
            mCurrentFile = 1;
        }

        LoadData();
        return true;

    case 'w':  // Toggle solid-wire mode.
    case 'W':
        if (mWireState == mEngine->GetRasterizerState())
        {
            mEngine->SetDefaultRasterizerState();
        }
        else
        {
            mEngine->SetRasterizerState(mWireState);
        }
        return true;
    }

    return Window::OnCharPress(key, x, y);
}

bool ConvexHull3DWindow::SetEnvironment()
{
    std::string path = mEnvironment.GetVariable("GTE_PATH");
    if (path == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Geometrics/ConvexHull3D/Data/");
    return true;
}

bool ConvexHull3DWindow::LoadData()
{
    std::string filename = "data";
    if (mCurrentFile < 10)
    {
        filename += "0";
    }
    filename += std::to_string(mCurrentFile) + ".txt";
    std::string path = mEnvironment.GetPath(filename);
    if (path == "")
    {
        return false;
    }

    std::ifstream input(path);
    if (!input)
    {
        return false;
    }

    Vector3<float> center{ 0.0f, 0.0f, 0.0f };
    unsigned int numVertices;
    input >> numVertices;
    std::vector<Vector3<float>> vertices(numVertices);
    for (auto& v : vertices)
    {
        for (int j = 0; j < 3; ++j)
        {
            input >> v[j];
        }
        center += v;
    }
    input.close();
    center /= static_cast<float>(numVertices);

    float radius = 0.0f;
    for (auto const& v : vertices)
    {
        Vector3<float> diff = v - center;
        float length = Length(diff);
        if (length > radius)
        {
            radius = length;
        }
    }

    // The worst-case number of words for UIntegerFP32<N> for 'float' input
    // to ConvexHull3 is N = 27.  For 'double', it is 'N = 197'.
    ConvexHull3<float, BSNumber<UIntegerFP32<27>>> ch;
    if (numVertices < 4 || !ch(numVertices, &vertices[0], 0.0f))
    {
        if (mMesh)
        {
            mTrackball.Detach(mMesh);
            mTrackball.Update();
            mCameraRig.Unsubscribe(mMesh->worldTransform);
            mMesh = nullptr;
        }

        mMessage = "File = " + std::to_string(mCurrentFile) +
            " has intrinsic dimension " + std::to_string(ch.GetDimension());
        return false;
    }
#if defined(GTE_COLLECT_BSNUMBER_STATISTICS)
    std::cout << "max size = " << gte::gBSNumberMaxSize << std::endl;
#endif

    std::vector<TriangleKey<true>> const& triangles = ch.GetHullUnordered();

    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.0f, 1.0f);

    struct Vertex
    {
        Vector3<float> position;
        Vector4<float> color;
    };
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);
    std::shared_ptr<VertexBuffer> vbuffer(new VertexBuffer(vformat,
        numVertices));
    Vertex* vertex = vbuffer->Get<Vertex>();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        vertex[i].position = vertices[i];
        vertex[i].color[0] = rnd(mte);
        vertex[i].color[1] = rnd(mte);
        vertex[i].color[2] = rnd(mte);
        vertex[i].color[3] = 1.0f;
    }

    unsigned int numPrimitives = static_cast<unsigned int>(triangles.size());
    std::shared_ptr<IndexBuffer> ibuffer(new IndexBuffer(IP_TRIMESH,
        numPrimitives, sizeof(unsigned int)));
    Memcpy(ibuffer->GetData(), &triangles[0], ibuffer->GetNumBytes());

    // Update all information associated with the mesh transforms.
    if (mMesh)
    {
        mTrackball.Detach(mMesh);
        mTrackball.Update();
        mCameraRig.Unsubscribe(mMesh->worldTransform);
    }
    mMesh = std::make_shared<Visual>(vbuffer, ibuffer, mEffect);
    mMesh->localTransform.SetTranslation(-center);
    mMesh->worldTransform = mMesh->localTransform;
    mCameraRig.Subscribe(mMesh->worldTransform,
        mEffect->GetPVWMatrixConstant());

    // Move the camera for a centered view of the mesh.
    Vector4<float> camPosition = Vector4<float>{0.0f, 0.0f, 0.0f, 1.0f}
        - 2.5f*radius*mCamera->GetDVector();
    mCamera->SetPosition(camPosition);

    // Update the message for display.
    mMessage =
        "File = " + std::to_string(mCurrentFile) + " , " +
        "Vertices = " + std::to_string(numVertices) + " , " +
        "Triangles =" + std::to_string(numPrimitives);

    mTrackball.Attach(mMesh);
    mTrackball.Update();
    mCameraRig.UpdatePVWMatrices();
    return true;
}

void ConvexHull3DWindow::InitializeCamera()
{
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 1.0f, 5000.0f);
    Vector4<float> camPosition{ 0.0f, 0.0f, -4.0f, 1.0f };
    Vector4<float> camDVector{ 0.0f, 0.0f, 1.0f, 0.0f };
    Vector4<float> camUVector{ 0.0f, 1.0f, 0.0f, 0.0f };
    Vector4<float> camRVector = Cross(camDVector, camUVector);
    mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

    mTranslationSpeed = 0.1f;
    mRotationSpeed = 0.01f;
    mTranslationSpeedFactor = 2.0f;
    mRotationSpeedFactor = 2.0f;
    mCameraRig.SetTranslationSpeed(mTranslationSpeed);
    mCameraRig.SetRotationSpeed(mRotationSpeed);
    mCameraRig.ComputeWorldAxes();
    mCameraRig.UpdatePVWMatrices();
}

