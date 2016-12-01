// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "MassSprings3DWindow.h"


MassSprings3DWindow::MassSprings3DWindow(Parameters& parameters)
    :
    Window3(parameters),
    mSimulationTime(0.0f),
    mSimulationDelta(0.001f)
{
    if (!SetEnvironment() || !CreateMassSpringSystem())
    {
        parameters.created = false;
        return;
    }

    mWireState = std::make_shared<RasterizerState>();
    mWireState->fillMode = RasterizerState::FILL_WIREFRAME;

    CreateBoxFaces();
    InitializeCamera();
}

void MassSprings3DWindow::OnIdle()
{
    mTimer.Measure();

    mCameraRig.Move();
    UpdateTransforms();
    UpdateMassSpringSystem();

    mEngine->ClearBuffers();
    for (int i = 0; i < 6; ++i)
    {
        mEngine->Draw(mBoxFace[i]);
    }
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool MassSprings3DWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mEngine->GetRasterizerState() != mWireState)
        {
            mEngine->SetRasterizerState(mWireState);
        }
        else
        {
            mEngine->SetDefaultRasterizerState();
        }
        return true;
    }
    return Window::OnCharPress(key, x, y);
}

bool MassSprings3DWindow::SetEnvironment()
{
    std::string path = mEnvironment.GetVariable("GTE_PATH");
    if (path == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return false;
    }
    mEnvironment.Insert(path + "/Samples/Physics/MassSprings3D/Shaders/");

    if (mEnvironment.GetPath("RungeKutta.hlsl") == "")
    {
        LogError("Cannot find RungeKutta.hlsl.");
        return false;
    }

    path = mEnvironment.GetPath("DrawUsingVertexID.hlsl");
    if (path == "")
    {
        LogError("Cannot find DrawUsingVertexID.hlsl.");
        return false;
    }

    return true;
}

void MassSprings3DWindow::InitializeCamera()
{
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 0.1f, 100.0f);
    Vector4<float> camPosition{ 4.0f, 0.0f, 0.0f, 1.0f };
    Vector4<float> camDVector{ -1.0f, 0.0f, 0.0f, 0.0f };
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

bool MassSprings3DWindow::CreateMassSpringSystem()
{
    // Size of mass-spring system.
    mDimension[0] = 32;
    mDimension[1] = 32;
    mDimension[2] = 32;

    // Viscous forces are applied.  If you set viscosity to zero, the cuboid
    // wiggles indefinitely since there is no dissipation of energy.  If the
    // viscosity is set to a positive value, the oscillations eventually stop.
    // The length of time to steady state is inversely proportional to the
    // viscosity.
    bool created = false;
    float viscosity = 0.1f;
    mMassSprings = std::make_unique<MassSpringSystem>(mProgramFactory,
        mDimension[0], mDimension[1], mDimension[2], mSimulationDelta,
        viscosity, mEnvironment, created);
    if (!created)
    {
        return false;
    }

    // The initial velocities are randomly generated.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(-1.0f, 1.0f);

    // The initial cuboid is axis-aligned.  The outer shell is immovable.
    // All other masses are constant.
    float cFactor = 2.0f / (mDimension[0] - 1.0f);
    float rFactor = 2.0f / (mDimension[1] - 1.0f);
    float sFactor = 2.0f / (mDimension[2] - 1.0f);
    int s, r, c;
    for (s = 0; s < mDimension[2]; ++s)
    {
        float z = -1.0f + s*sFactor;
        for (r = 0; r < mDimension[1]; ++r)
        {
            float y = -1.0f + r*rFactor;
            for (c = 0; c < mDimension[0]; ++c)
            {
                float x = -1.0f + c*cFactor;
                mMassSprings->SetPosition(c, r, s, { x, y, z });

                if (1 <= s && s < mDimension[2] - 1
                &&  1 <= r && r < mDimension[1] - 1
                &&  1 <= c && c < mDimension[0] - 1)
                {
                    mMassSprings->SetMass(c, r, s, 1.0f);
                    mMassSprings->SetVelocity(c, r, s,
                        { 0.1f*rnd(mte), 0.1f*rnd(mte), 0.1f*rnd(mte) });
                }
                else
                {
                    mMassSprings->SetMass(c, r, s,
                        std::numeric_limits<float>::max());
                    mMassSprings->SetVelocity(c, r, s, { 0.0f, 0.0f, 0.0f });
                }
            }
        }
    }

    // Springs are at rest in the initial configuration.
    float const constant = 10.0f;
    Vector3<float> diff;

    for (s = 0; s < mDimension[2] - 1; ++s)
    {
        for (r = 0; r < mDimension[1]; ++r)
        {
            for (c = 0; c < mDimension[0]; ++c)
            {
                mMassSprings->SetConstantS(c, r, s, constant);
                diff = mMassSprings->GetPosition(c, r, s + 1) -
                    mMassSprings->GetPosition(c, r, s);
                mMassSprings->SetLengthS(c, r, s, Length(diff));
            }
        }
    }

    for (s = 0; s < mDimension[2]; ++s)
    {
        for (r = 0; r < mDimension[1] - 1; ++r)
        {
            for (c = 0; c < mDimension[0]; ++c)
            {
                mMassSprings->SetConstantR(c, r, s, constant);
                diff = mMassSprings->GetPosition(c, r + 1, s) -
                    mMassSprings->GetPosition(c, r, s);
                mMassSprings->SetLengthR(c, r, s, Length(diff));
            }
        }
    }

    for (s = 0; s < mDimension[2]; ++s)
    {
        for (r = 0; r < mDimension[1]; ++r)
        {
            for (c = 0; c < mDimension[0] - 1; ++c)
            {
                mMassSprings->SetConstantC(c, r, s, constant);
                diff = mMassSprings->GetPosition(c + 1, r, s) -
                    mMassSprings->GetPosition(c, r, s);
                mMassSprings->SetLengthC(c, r, s, Length(diff));
            }
        }
    }

    return true;
}

void MassSprings3DWindow::CreateBoxFaces()
{
    // The vertex buffer will use the mass-spring position array for its
    // CPU data.
    int const numVertices = mDimension[0] * mDimension[1] * mDimension[2];

#if defined(DO_CPU_MASS_SPRING)
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    mVBuffer = std::make_shared<VertexBuffer>(vformat, numVertices, false);
    mVBuffer->SetUsage(Resource::DYNAMIC_UPDATE);
#else
    mVBuffer = std::make_shared<VertexBuffer>(numVertices);
#endif
    
    size_t const idxsize = sizeof(unsigned int);
    std::shared_ptr<IndexBuffer> ibuffer;
    int numTriangles, t, x, y, z, v0, v1, v2, v3;

    // box face z = 1
    numTriangles = 2 * (mDimension[0] - 1) * (mDimension[1] - 1);
    ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles,
        idxsize);
    for (y = 1, t = 0; y < mDimension[1] - 2; ++y)
    {
        for (x = 1; x < mDimension[0] - 2; ++x)
        {
            v0 = GetIndex(x, y, 1);
            v1 = GetIndex(x + 1, y, 1);
            v2 = GetIndex(x, y + 1, 1);
            v3 = GetIndex(x + 1, y + 1, 1);
            ibuffer->SetTriangle(t++, v0, v2, v3);
            ibuffer->SetTriangle(t++, v0, v3, v1);
        }
    }
    mBoxFace[0] = std::make_shared<Visual>(mVBuffer, ibuffer);

    // box face z = dim2 - 2
    numTriangles = 2 * (mDimension[0] - 1) * (mDimension[1] - 1);
    ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles,
        idxsize);
    for (y = 1, t = 0; y < mDimension[1] - 2; ++y)
    {
        for (x = 1; x < mDimension[0] - 2; ++x)
        {
            v0 = GetIndex(x, y, mDimension[2] - 2);
            v1 = GetIndex(x + 1, y, mDimension[2] - 2);
            v2 = GetIndex(x, y + 1, mDimension[2] - 2);
            v3 = GetIndex(x + 1, y + 1, mDimension[2] - 2);
            ibuffer->SetTriangle(t++, v0, v3, v2);
            ibuffer->SetTriangle(t++, v0, v1, v3);
        }
    }
    mBoxFace[1] = std::make_shared<Visual>(mVBuffer, ibuffer);

    // box face y = 1
    numTriangles = 2 * (mDimension[0] - 1) * (mDimension[2] - 1);
    ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles,
        idxsize);
    for (z = 1, t = 0; z < mDimension[2] - 2; ++z)
    {
        for (x = 1; x < mDimension[0] - 2; ++x)
        {
            v0 = GetIndex(x, 1, z);
            v1 = GetIndex(x + 1, 1, z);
            v2 = GetIndex(x, 1, z + 1);
            v3 = GetIndex(x + 1, 1, z + 1);
            ibuffer->SetTriangle(t++, v0, v3, v2);
            ibuffer->SetTriangle(t++, v0, v1, v3);
        }
    }
    mBoxFace[2] = std::make_shared<Visual>(mVBuffer, ibuffer);

    // box face y = dim1 - 2
    numTriangles = 2 * (mDimension[0] - 1) * (mDimension[2] - 1);
    ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles,
        idxsize);
    for (z = 1, t = 0; z < mDimension[2] - 2; ++z)
    {
        for (x = 1; x < mDimension[0] - 2; ++x)
        {
            v0 = GetIndex(x, mDimension[1] - 2, z);
            v1 = GetIndex(x + 1, mDimension[1] - 2, z);
            v2 = GetIndex(x, mDimension[1] - 2, z + 1);
            v3 = GetIndex(x + 1, mDimension[1] - 2, z + 1);
            ibuffer->SetTriangle(t++, v0, v2, v3);
            ibuffer->SetTriangle(t++, v0, v3, v1);
        }
    }
    mBoxFace[3] = std::make_shared<Visual>(mVBuffer, ibuffer);

    // box face x = 1
    numTriangles = 2 * (mDimension[1] - 1) * (mDimension[2] - 1);
    ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles,
        idxsize);
    for (z = 1, t = 0; z < mDimension[2] - 2; ++z)
    {
        for (y = 1; y < mDimension[1] - 2; ++y)
        {
            v0 = GetIndex(1, y, z);
            v1 = GetIndex(1, y + 1, z);
            v2 = GetIndex(1, y, z + 1);
            v3 = GetIndex(1, y + 1, z + 1);
            ibuffer->SetTriangle(t++, v0, v2, v3);
            ibuffer->SetTriangle(t++, v0, v3, v1);
        }
    }
    mBoxFace[4] = std::make_shared<Visual>(mVBuffer, ibuffer);

    // box face x = dim0 - 2
    numTriangles = 2 * (mDimension[1] - 1) * (mDimension[2] - 1);
    ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles,
        idxsize);
    for (z = 1, t = 0; z < mDimension[2] - 2; ++z)
    {
        for (y = 1; y < mDimension[1] - 2; ++y)
        {
            v0 = GetIndex(mDimension[0] - 2, y, z);
            v1 = GetIndex(mDimension[0] - 2, y + 1, z);
            v2 = GetIndex(mDimension[0] - 2, y, z + 1);
            v3 = GetIndex(mDimension[0] - 2, y + 1, z + 1);
            ibuffer->SetTriangle(t++, v0, v3, v2);
            ibuffer->SetTriangle(t++, v0, v1, v3);
        }
    }
    mBoxFace[5] = std::make_shared<Visual>(mVBuffer, ibuffer);

    Vector4<float> const color[6] =
    {
        { 1.0f, 0.0f, 0.0f, 1.0f },
        { 0.0f, 1.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f, 1.0f },
        { 0.0f, 1.0f, 1.0f, 1.0f },
        { 1.0f, 0.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 0.0f, 1.0f }
    };

#if defined(DO_CPU_MASS_SPRING)
    for (int i = 0; i < 6; ++i)
    {
        mEffect[i] = std::make_shared<ConstantColorEffect>(color[i]);
        mBoxFace[i]->SetEffect(mEffect[i]);
    }
#else
    std::string path = mEnvironment.GetPath("DrawUsingVertexID.hlsl");
    std::shared_ptr<ConstantBuffer> pvwMatrixBuffer;
    std::shared_ptr<ConstantBuffer> colorBuffer;
    std::shared_ptr<VisualProgram> program;
    std::shared_ptr<VertexShader> vshader;
    std::shared_ptr<PixelShader> pshader;
    for (int i = 0; i < 6; ++i)
    {
        pvwMatrixBuffer = std::make_shared<ConstantBuffer>(
            sizeof(Matrix4x4<float>), true);
        colorBuffer = std::make_shared<ConstantBuffer>(
            sizeof(Vector4<float>), false);
        *colorBuffer->Get<Vector4<float>>() = color[i];
        // TODO: Need to terminate application if 'program' is null.
        program = mProgramFactory.CreateFromFiles(path, path, "");
        vshader = program->GetVShader();
        vshader->Set("PVWMatrix", pvwMatrixBuffer);
        vshader->Set("ConstantColor", colorBuffer);
        vshader->Set("position", mMassSprings->GetPosition());
        mEffect[i] = std::make_shared<VisualEffect>(program);
        mBoxFace[i]->SetEffect(mEffect[i]);
    }
#endif
}

void MassSprings3DWindow::UpdateTransforms()
{
    Matrix4x4<float> pvMatrix = mCamera->GetProjectionViewMatrix();
    for (int i = 0; i < 6; ++i)
    {
        Matrix4x4<float> pvwMatrix;
#if defined(GTE_USE_MAT_VEC)
        pvwMatrix = pvMatrix * mTrackball.GetOrientation();
#else
        pvwMatrix = mTrackball.GetOrientation() * pvMatrix;
#endif

        std::shared_ptr<ConstantBuffer> cbuffer =
            mEffect[i]->GetVertexShader()->Get<ConstantBuffer>("PVWMatrix");
        *cbuffer->Get<Matrix4x4<float>>() = pvwMatrix;
        mEngine->Update(cbuffer);
    }
}

void MassSprings3DWindow::UpdateMassSpringSystem()
{
#if defined(DO_CPU_MASS_SPRING)
    mMassSprings->Update(mSimulationTime);

    std::vector<Vector3<float>> const& vertices = mMassSprings->GetPosition();
    mVBuffer->SetData((char*)&vertices[0]);
    mEngine->Update(mVBuffer);
    mVBuffer->ResetData();
#else
    mMassSprings->Update(mSimulationTime, mEngine);
#endif

    mSimulationTime += mSimulationDelta;
}

