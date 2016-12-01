// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "Interpolation2DWindow.h"


Interpolation2DWindow::Interpolation2DWindow(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    CreateCommonObjects();
    CreateBilinearMesh();
    OnDisplay();
}

void Interpolation2DWindow::OnDisplay()
{
    mCameraRig.Move();
    mEngine->ClearBuffers();
    mEngine->Draw(mMesh);
    mEngine->Draw(8, 24, { 0.0f, 0.0f, 0.0f, 1.0f }, mName);
    mEngine->DisplayColorBuffer(0);
}

void Interpolation2DWindow::OnIdle()
{
    if (mCameraRig.Move())
    {
        OnDisplay();
    }
}

bool Interpolation2DWindow::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case '0':
        if (mName != "Bilinear2")
        {
            CreateBilinearMesh();
            OnDisplay();
        }
        return true;
    case '1':
        if (mName != "Bicubic2_CM")
        {
            CreateBicubicMesh(true);
            OnDisplay();
        }
        return true;
    case '2':
        if (mName != "Bicubic2_BS")
        {
            CreateBicubicMesh(false);
            OnDisplay();
        }
        return true;
    case '3':
        if (mName != "AkimaUniform2_BS")
        {
            CreateAkimaUniformMesh();
            OnDisplay();
        }
        return true;
    case '4':
        if (mName != "ThinPlateSpline2_0")
        {
            CreateThinPlateSplineMesh(0.0f);
            OnDisplay();
        }
        return true;
    case '5':
        if (mName != "ThinPlateSpline2_1")
        {
            CreateThinPlateSplineMesh(1.0f);
            OnDisplay();
        }
        return true;
    case '6':
        if (mName != "ThinPlateSpline2_10")
        {
            CreateThinPlateSplineMesh(10.0f);
            OnDisplay();
        }
        return true;
    case '7':
        if (mName != "LinearNonuniform")
        {
            CreateLinearNonuniform();
            OnDisplay();
        }
        return true;
    case '8':
        if (mName != "QuadraticNonuniform_NOGRAD")
        {
            CreateQuadraticNonuniform(false);
            OnDisplay();
        }
        return true;
    case '9':
        if (mName != "QuadraticNonuniform_GRAD")
        {
            CreateQuadraticNonuniform(true);
            OnDisplay();
        }
        return true;
    case 'w':
    case 'W':
        if (mEngine->GetRasterizerState() == mNoCullSolidState)
        {
            mEngine->SetRasterizerState(mNoCullWireState);
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullSolidState);
        }
        OnDisplay();
        return true;
    }

    return Window::OnCharPress(key, x, y);
}

bool Interpolation2DWindow::OnMouseClick(MouseButton button,
    MouseState state, int x, int y, unsigned int modifiers)
{
    if (Window3::OnMouseClick(button, state, x, y, modifiers))
    {
        OnDisplay();
    }
    return true;
}

bool Interpolation2DWindow::OnMouseMotion(MouseButton button, int x,
    int y, unsigned int modifiers)
{
    if (Window3::OnMouseMotion(button, x, y, modifiers))
    {
        OnDisplay();
    }
    return true;
}

bool Interpolation2DWindow::SetEnvironment()
{
    std::string gtpath = mEnvironment.GetVariable("GTE_PATH");
    if (gtpath == "")
    {
        LogError("You must create the environment variable GTE_PATH.");
        return false;
    }
    mEnvironment.Insert(gtpath + "/Samples/Data/");

    if (mEnvironment.GetPath("Checkerboard.png") == "")
    {
        LogError("Cannot find file Checkerboard.png.");
        return false;
    }

    return true;
}

void Interpolation2DWindow::InitializeCamera()
{
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 0.1f, 100.0f);
    Vector4<float> camPosition{ 0.0f, 0.0f, 2.5f, 1.0f };
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

void Interpolation2DWindow::CreateCommonObjects()
{
    InitializeCamera();

    std::string path = mEnvironment.GetPath("Checkerboard.png");
    mTexture.reset(WICFileIO::Load(path, true));
    mTexture->AutogenerateMipmaps();
    mEffect = std::make_shared<Texture2Effect>(mProgramFactory, mTexture,
        SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::CLAMP,
        SamplerState::CLAMP);

    mNoCullSolidState = std::make_shared<RasterizerState>();
    mNoCullSolidState->cullMode = RasterizerState::CULL_NONE;
    mNoCullSolidState->fillMode = RasterizerState::FILL_SOLID;
    mEngine->SetRasterizerState(mNoCullSolidState);

    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cullMode = RasterizerState::CULL_NONE;
    mNoCullWireState->fillMode = RasterizerState::FILL_WIREFRAME;

    float const maxRnd = 0.125f;
    std::mt19937 mte;
    std::uniform_real<float> rnd(0.0f, maxRnd);
    mFSample.resize(SAMPLE_BOUNDSQR);
    for (int y = 0; y < SAMPLE_BOUND; ++y)
    {
        for (int x = 0; x < SAMPLE_BOUND; ++x)
        {
            mFSample[x + SAMPLE_BOUND*y] = rnd(mte);
        }
    }

    mF[0] = 0.0f;   mDFDX[0] = -0.125f;     mDFDY[0] = -0.125f;
    mF[1] = 1.0f;   mDFDX[1] = 0.0f;        mDFDY[1] = -0.25f;
    mF[2] = 0.5f;   mDFDX[2] = 0.25f;       mDFDY[2] = 0.0f;
    mF[3] = 1.0f;   mDFDX[3] = -0.0625f;    mDFDY[3] = 0.0625f;
    mF[4] = 1.0f;   mDFDX[4] = 0.0625f;     mDFDY[4] = -0.0625f;
    mF[5] = 0.25f;  mDFDX[5] = 0.0f;        mDFDY[5] = -0.25f;

    std::fill(mDFDX.begin(), mDFDX.end(), 0.0f);
    std::fill(mDFDY.begin(), mDFDY.end(), 0.0f);
}

void Interpolation2DWindow::OnPrecreateMesh()
{
    if (mMesh)
    {
        mTrackball.Detach(mMesh);
        mCameraRig.Unsubscribe(mMesh->worldTransform);
    }
}

void Interpolation2DWindow::OnPostcreateMesh()
{
    mMesh->SetEffect(mEffect);
    mCameraRig.Subscribe(mMesh->worldTransform,
        mEffect->GetPVWMatrixConstant());

    mTrackball.Attach(mMesh);
    mTrackball.Update();
    mCameraRig.UpdatePVWMatrices();
}

void Interpolation2DWindow::CreateBilinearMesh()
{
    OnPrecreateMesh();

    mName = "Bilinear2";

    float const spacing = 2.0f / static_cast<float>(SAMPLE_BOUND - 1);
    IntpBilinear2<float> interp(SAMPLE_BOUND, SAMPLE_BOUND, -1.0f, spacing,
        -1.0f, spacing, &mFSample[0]);

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    int const numSamples = 64;
    mMesh = mf.CreateRectangle(numSamples, numSamples, 1.0f, 1.0f);
    Vertex* vertices = mMesh->GetVertexBuffer()->Get<Vertex>();
    for (int y = 0, i = 0; y < numSamples; ++y)
    {
        float fy = vertices[i].position[1];
        for (int x = 0; x < numSamples; ++x, ++i)
        {
            float fx = vertices[i].position[0];
            vertices[i].position[2] = interp(fx, fy);
        }
    }

    OnPostcreateMesh();
}

void Interpolation2DWindow::CreateBicubicMesh(bool catmullRom)
{
    OnPrecreateMesh();

    if (catmullRom)
    {
        mName = "Bicubic2_CR";
    }
    else
    {
        mName = "Bicubic2_BS";
    }

    float const spacing = 2.0f / static_cast<float>(SAMPLE_BOUND - 1);
    IntpBicubic2<float> interp(SAMPLE_BOUND, SAMPLE_BOUND, -1.0f, spacing,
        -1.0f, spacing, &mFSample[0], catmullRom);

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    int const numSamples = 64;
    mMesh = mf.CreateRectangle(numSamples, numSamples, 1.0f, 1.0f);
    Vertex* vertices = mMesh->GetVertexBuffer()->Get<Vertex>();
    for (int y = 0, i = 0; y < numSamples; ++y)
    {
        float fy = vertices[i].position[1];
        for (int x = 0; x < numSamples; ++x, ++i)
        {
            float fx = vertices[i].position[0];
            vertices[i].position[2] = interp(fx, fy);
        }
    }

    OnPostcreateMesh();
}

void Interpolation2DWindow::CreateAkimaUniformMesh()
{
    OnPrecreateMesh();

    mName = "AkimaUniform2";

    float const spacing = 2.0f / static_cast<float>(SAMPLE_BOUND - 1);
    IntpAkimaUniform2<float> interp(SAMPLE_BOUND, SAMPLE_BOUND, -1.0f,
        spacing, -1.0f, spacing, &mFSample[0]);

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    int const numSamples = 64;
    mMesh = mf.CreateRectangle(numSamples, numSamples, 1.0f, 1.0f);
    Vertex* vertices = mMesh->GetVertexBuffer()->Get<Vertex>();
    for (int y = 0, i = 0; y < numSamples; ++y)
    {
        float fy = vertices[i].position[1];
        for (int x = 0; x < numSamples; ++x, ++i)
        {
            float fx = vertices[i].position[0];
            vertices[i].position[2] = interp(fx, fy);
        }
    }

    OnPostcreateMesh();
}

void Interpolation2DWindow::CreateThinPlateSplineMesh(float smooth)
{
    OnPrecreateMesh();

    if (smooth == 0.0f)
    {
        mName = "ThinPlateSpline2_0";
    }
    else if (smooth == 1.0f)
    {
        mName = "ThinPlateSpline2_1";
    }
    else
    {
        mName = "ThinPlateSpline2_10";
    }

    float const spacing = 2.0f / static_cast<float>(SAMPLE_BOUND - 1);
    std::vector<float> xDomain(SAMPLE_BOUNDSQR);
    std::vector<float> yDomain(SAMPLE_BOUNDSQR);
    for (int y = 0, i = 0; y < SAMPLE_BOUND; ++y)
    {
        float fy = -1.0f + spacing * y;
        for (int x = 0; x < SAMPLE_BOUND; ++x, ++i)
        {
            float fx = -1.0f + spacing * x;
            xDomain[i] = fx;
            yDomain[i] = fy;
        }
    }

    IntpThinPlateSpline2<float> interp(SAMPLE_BOUNDSQR, &xDomain[0],
        &yDomain[0], &mFSample[0], smooth, false);

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    int const numSamples = 64;
    mMesh = mf.CreateRectangle(numSamples, numSamples, 1.0f, 1.0f);
    Vertex* vertices = mMesh->GetVertexBuffer()->Get<Vertex>();
    for (int y = 0, i = 0; y < numSamples; ++y)
    {
        float fy = vertices[i].position[1];
        for (int x = 0; x < numSamples; ++x, ++i)
        {
            float fx = vertices[i].position[0];
            vertices[i].position[2] = interp(fx, fy);
        }
    }

    OnPostcreateMesh();
}

void Interpolation2DWindow::CreateLinearNonuniform()
{
    OnPrecreateMesh();

    mName = "LinearNonuniform";

    IntpLinearNonuniform2<float, SimpleMesh> interp(mSimpleMesh, &mF[0]);

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    int const numSamples = 64;
    mMesh = mf.CreateTriangle(numSamples, 1.0f, 1.0f);
    Vertex* vertices = mMesh->GetVertexBuffer()->Get<Vertex>();
    Vector2<float> P;
    float F;
    for (int y = 0, i = 0; y < numSamples; ++y)
    {
        for (int x = 0; x + y < numSamples; ++x, ++i)
        {
            // For the sake of the demonstration, ensure P is inside the
            // domain of the interpolator for all (x,y).
            P[0] = std::min(std::max(vertices[i].position[0], 0.0f), 1.0f);
            P[1] = std::min(std::max(vertices[i].position[1], 0.0f), 1.0f);
            float sum = P[0] + P[1];
            if (sum > 1.0f)
            {
                P[0] /= sum;
                P[1] /= sum;
            }

            if (interp(P, F))
            {
                vertices[i].position[2] = F;
            }
            else
            {
                // We should not get here with the adjustment above.
                vertices[i].position[2] = 0.0f;
            }
        }
    }

    OnPostcreateMesh();
}

void Interpolation2DWindow::CreateQuadraticNonuniform(bool useGradients)
{
    OnPrecreateMesh();

    IntpQuadraticNonuniform2<float, SimpleMesh>* interp;
    if (useGradients)
    {
        mName = "QuadraticNonuniform_GRAD";
        interp = new IntpQuadraticNonuniform2<float, SimpleMesh>(mSimpleMesh,
            &mF[0], &mDFDX[0], &mDFDY[0]);
    }
    else
    {
        mName = "QuadraticNonuniform_NOGRAD";
        float spatialDelta = 0.125f;
        interp = new IntpQuadraticNonuniform2<float, SimpleMesh>(mSimpleMesh,
            &mF[0], spatialDelta);
    }

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    int const numSamples = 128;
    mMesh = mf.CreateTriangle(numSamples, 1.0f, 1.0f);
    Vertex* vertices = mMesh->GetVertexBuffer()->Get<Vertex>();
    Vector2<float> P;
    float F, FX, FY;
    int i = 0;
    for (int y = 0; y < numSamples; ++y)
    {
        for (int x = 0; x + y < numSamples; ++x, ++i)
        {
            // For the sake of the demonstration, ensure P is inside the
            // domain of the interpolator for all (x,y).
            P[0] = std::min(std::max(vertices[i].position[0], 0.0f), 1.0f);
            P[1] = std::min(std::max(vertices[i].position[1], 0.0f), 1.0f);
            float sum = P[0] + P[1];
            if (sum > 1.0f)
            {
                P[0] /= sum;
                P[1] /= sum;
            }

            if ((*interp)(P, F, FX, FY))
            {
                vertices[i].position[2] = F;
            }
            else
            {
                // We should not get here with the adjustment above.
                vertices[i].position[2] = 0.0f;
            }
        }
    }
    delete interp;

    OnPostcreateMesh();
}

Interpolation2DWindow::SimpleMesh::SimpleMesh()
{
    mVertices[0] = { 0.0f, 0.0f };
    mVertices[1] = { 0.5f, 0.0f };
    mVertices[2] = { 1.0f, 0.0f };
    mVertices[3] = { 0.0f, 0.5f };
    mVertices[4] = { 0.5f, 0.5f };
    mVertices[5] = { 0.0f, 1.0f };

    mIndices[0] = 0;  mIndices[1] = 1;  mIndices[2] = 3;
    mIndices[3] = 1;  mIndices[4] = 2;  mIndices[5] = 4;
    mIndices[6] = 3;  mIndices[7] = 4;  mIndices[8] = 5;
    mIndices[9] = 1;  mIndices[10] = 4;  mIndices[11] = 3;

    mAdjacencies[0] = -1;  mAdjacencies[1] = 3;  mAdjacencies[2] = -1;
    mAdjacencies[3] = -1;  mAdjacencies[4] = -1;  mAdjacencies[5] = 3;
    mAdjacencies[6] = 3;  mAdjacencies[7] = -1;  mAdjacencies[8] = -1;
    mAdjacencies[9] = 0;  mAdjacencies[10] = 1;  mAdjacencies[11] = 2;
}

int Interpolation2DWindow::SimpleMesh::GetNumVertices() const
{
    return 6;
}

int Interpolation2DWindow::SimpleMesh::GetNumTriangles() const
{
    return 4;
}

Vector2<float> const* Interpolation2DWindow::SimpleMesh::GetVertices() const
{
    return &mVertices[0];
}

int const* Interpolation2DWindow::SimpleMesh::GetIndices() const
{
    return &mIndices[0];
}

bool Interpolation2DWindow::SimpleMesh::GetVertices(int t,
    std::array<Vector2<float>, 3>& vertices) const
{
    std::array<int, 3> indices;
    if (GetIndices(t, indices))
    {
        vertices[0] = mVertices[indices[0]];
        vertices[1] = mVertices[indices[1]];
        vertices[2] = mVertices[indices[2]];
        return true;
    }
    return false;
}

bool Interpolation2DWindow::SimpleMesh::GetIndices(int t,
    std::array<int, 3>& indices) const
{
    if (0 <= t && t < 4)
    {
        indices[0] = mIndices[3 * t + 0];
        indices[1] = mIndices[3 * t + 1];
        indices[2] = mIndices[3 * t + 2];
        return true;
    }
    return false;
}

bool Interpolation2DWindow::SimpleMesh::GetAdjacencies(int t,
    std::array<int, 3>& adjacencies) const
{
    if (0 <= t && t < 4)
    {
        adjacencies[0] = mAdjacencies[3 * t + 0];
        adjacencies[1] = mAdjacencies[3 * t + 1];
        adjacencies[2] = mAdjacencies[3 * t + 2];
        return true;
    }
    return false;
}

bool Interpolation2DWindow::SimpleMesh::GetBarycentrics(int t,
    Vector2<float> const& P, std::array<float, 3>& bary) const
{
    std::array<Vector2<float>, 3> V;
    if (GetVertices(t, V))
    {
        return ComputeBarycentrics(P, V[0], V[1], V[2], &bary[0], 0.0f);
    }
    return false;
}

int Interpolation2DWindow::SimpleMesh::GetContainingTriangle(
    Vector2<float> const& P) const
{
    if (P[0] < 0.0f || P[1] < 0.0f || P[0] + P[1] > 1.0f)
    {
        return -1;
    }

    if (P[0] + P[1] <= 0.5f)
    {
        return 0;
    }

    if (P[0] >= 0.5f)
    {
        return 1;
    }

    if (P[1] >= 0.5f)
    {
        return 2;
    }

    return 3;
}

