// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
using namespace gte;

class Interpolation2DWindow : public Window3
{
public:
    Interpolation2DWindow(Parameters& parameters);

    virtual void OnDisplay();
    virtual void OnIdle();
    virtual bool OnCharPress(unsigned char key, int x, int y);
    virtual bool OnMouseClick(MouseButton button, MouseState state,
        int x, int y, unsigned int modifiers);
    virtual bool OnMouseMotion(MouseButton button, int x, int y,
        unsigned int modifiers);

private:
    bool SetEnvironment();
    void InitializeCamera();
    void CreateCommonObjects();
    void OnPrecreateMesh();
    void OnPostcreateMesh();
    void CreateBilinearMesh();
    void CreateBicubicMesh(bool catmullRom);
    void CreateAkimaUniformMesh();
    void CreateThinPlateSplineMesh(float smooth);
    void CreateLinearNonuniform();
    void CreateQuadraticNonuniform(bool useGradients);

    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    std::string mName;
    std::shared_ptr<Visual> mMesh;
    std::shared_ptr<Texture2> mTexture;
    std::shared_ptr<Texture2Effect> mEffect;
    std::shared_ptr<RasterizerState> mNoCullSolidState;
    std::shared_ptr<RasterizerState> mNoCullWireState;

    // For use by Bilinear, Bicubic, Akima, ThinPlateSpline.  The grid size
    // is SAMPLE_BOUND-by-SAMPLE_BOUND.
    enum
    {
        SAMPLE_BOUND = 8,
        SAMPLE_BOUNDSQR = SAMPLE_BOUND * SAMPLE_BOUND
    };
    std::vector<float> mFSample;
    typedef BSNumber<UIntegerAP32> Numeric;
    typedef BSRational<UIntegerAP32> Rational;
    typedef Delaunay2Mesh<float, Numeric, Rational> TriangleMesh;
    Delaunay2<float, Numeric> mDelaunay;

    // For use by LinearNonuniform, QuadraticNonuniform.
    class SimpleMesh
    {
    public:
        SimpleMesh();

        int GetNumVertices() const;
        int GetNumTriangles() const;
        Vector2<float> const* GetVertices() const;
        int const* GetIndices() const;
        bool GetVertices(int t,
            std::array<Vector2<float>, 3>& vertices) const;
        bool GetIndices(int t, std::array<int, 3>& indices) const;
        bool GetAdjacencies(int t, std::array<int, 3>& adjacencies) const;
        bool GetBarycentrics(int t, Vector2<float> const& P,
            std::array<float, 3>& bary) const;
        int GetContainingTriangle(Vector2<float> const& P) const;
    private:
        std::array<Vector2<float>, 6> mVertices;
        std::array<int, 12> mIndices;
        std::array<int, 12> mAdjacencies;
    };

    SimpleMesh mSimpleMesh;
    std::array<float, 6> mF, mDFDX, mDFDY;
};
