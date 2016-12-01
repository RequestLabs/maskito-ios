// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <Applications/GteWICFileIO.h>
#include "SimpleBumpMapEffect.h"
using namespace gte;


SimpleBumpMapEffect::SimpleBumpMapEffect(ProgramFactory& factory,
    Environment const& environment, bool& created)
    :
    mPVWMatrix(nullptr)
{
    created = false;

    // Load and compile the shaders.
    std::string path = environment.GetPath("SimpleBumpMap.hlsl");
#if !defined(GTE_DEV_OPENGL)
    // The flags are chosen to allow you to debug the shaders through MSVS.
    // The menu path is "Debug | Graphics | Start Diagnostics" (ALT+F5).
    factory.PushFlags();
    factory.flags =
        D3DCOMPILE_ENABLE_STRICTNESS |
        D3DCOMPILE_IEEE_STRICTNESS |
        D3DCOMPILE_DEBUG |
        D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    mProgram = factory.CreateFromFiles(path, path, "");
#if !defined(GTE_DEV_OPENGL)
    factory.PopFlags();
#endif
    if (!mProgram)
    {
        // The program factory will generate Log* messages.
        return;
    }

    // Load the textures.
    path = environment.GetPath("Bricks.png");
    mBaseTexture.reset(WICFileIO::Load(path, true));
    mBaseTexture->AutogenerateMipmaps();

    path = environment.GetPath("BricksNormal.png");
    mNormalTexture.reset(WICFileIO::Load(path, true));
    mNormalTexture->AutogenerateMipmaps();

    // Create the shader constants.
    mPVWMatrixConstant =
        std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    mPVWMatrix = mPVWMatrixConstant->Get<Matrix4x4<float>>();
    *mPVWMatrix = Matrix4x4<float>::Identity();

    // Create the texture sampler for mipmapping.
    mCommonSampler = std::make_shared<SamplerState>();
    mCommonSampler->filter = SamplerState::MIN_L_MAG_L_MIP_L;
    mCommonSampler->mode[0] = SamplerState::WRAP;
    mCommonSampler->mode[1] = SamplerState::WRAP;

    // Set the resources for the shaders.
    std::shared_ptr<VertexShader> vshader = mProgram->GetVShader();
    std::shared_ptr<PixelShader> pshader = mProgram->GetPShader();
    vshader->Set("PVWMatrix", mPVWMatrixConstant);
    pshader->Set("baseTexture", mBaseTexture);
    pshader->Set("normalTexture", mNormalTexture);
    pshader->Set("commonSampler", mCommonSampler);

    created = true;
}

void SimpleBumpMapEffect::ComputeLightVectors(std::shared_ptr<Visual> const& mesh,
    Vector4<float> const& worldLightDirection)
{
    // The tangent-space coordinates for the light direction vector at each
    // vertex is stored in the color0 channel.  The computations use the
    // vertex normals and the texture coordinates for the base mesh, which
    // are stored in the tcoord0 channel.  Thus, the mesh must have positions,
    // normals, colors (unit 0), and texture coordinates (unit 0).  The struct
    // shown next is consistent with mesh->GetVertexFormat().
    struct Vertex
    {
        Vector3<float> position;
        Vector3<float> normal;
        Vector3<float> lightDirection;
        Vector2<float> baseTCoord;
        Vector2<float> normalTCoord;
    };

    // The light direction D is in world-space coordinates.  Negate it,
    // transform it to model-space coordinates, and then normalize it.  The
    // world-space direction is unit-length, but the geometric primitive
    // might have non-unit scaling in its model-to-world transformation, in
    // which case the normalization is necessary.
#if defined(GTE_USE_MAT_VEC)
    Vector4<float> tempDirection =
        -(mesh->worldTransform.GetHInverse() * worldLightDirection);
    Vector3<float> modelLightDirection = HProject(tempDirection);
#else
    // TODO: Test this case.
    Vector4<float> tempDirection =
        -(worldLightDirection * mesh->worldTransform.GetHInverse());
    Vector3<float> modelLightDirection = HProject(tempDirection);
#endif

    // Set the light vectors to (0,0,0) as a flag that the quantity has not
    // yet been computed.  The probability that a light vector is actually
    // (0,0,0) should be small, so the flag system should save computation
    // time overall.
    std::shared_ptr<VertexBuffer> vbuffer = mesh->GetVertexBuffer();
    unsigned int const numVertices = vbuffer->GetNumElements();
    Vertex* vertex = vbuffer->Get<Vertex>();
    Vector3<float> const zero{ 0.0f, 0.0f, 0.0f };
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        vertex[i].lightDirection = zero;
    }

    std::shared_ptr<IndexBuffer> ibuffer = mesh->GetIndexBuffer();
    unsigned int numTriangles = ibuffer->GetNumPrimitives();
    for (unsigned int t = 0; t < numTriangles; ++t)
    {
        // Get the triangle vertices and attributes.
        unsigned int v0, v1, v2;
        if (!ibuffer->GetTriangle(t, v0, v1, v2))
        {
            continue;
        }

        Vector3<float> position[3] =
        {
            vertex[v0].position,
            vertex[v1].position,
            vertex[v2].position
        };

        Vector3<float> normal[3] =
        {
            vertex[v0].normal,
            vertex[v1].normal,
            vertex[v2].normal
        };

        Vector3<float>* direction[3] =
        {
            &vertex[v0].lightDirection,
            &vertex[v1].lightDirection,
            &vertex[v2].lightDirection
        };

        Vector2<float> tcoord[3] =
        {
            vertex[v0].baseTCoord,
            vertex[v1].baseTCoord,
            vertex[v2].baseTCoord
        };

        for (int i = 0; i < 3; ++i)
        {
            Vector3<float>& currentDirection = *direction[i];
            if (currentDirection != zero)
            {
                continue;
            }

            int iP = (i == 0) ? 2 : i - 1;
            int iN = (i + 1) % 3;

            Vector3<float> tangent;
            if (!ComputeTangent(position[i], tcoord[i], position[iN],
                tcoord[iN], position[iP], tcoord[iP], tangent))
            {
                // The texture coordinate mapping is not properly defined for
                // this.  Just say that the tangent space light vector points
                // in the same direction as the surface normal.
                currentDirection = normal[i];
                continue;
            }

            // Project T into the tangent plane by projecting out the surface
            // normal N, and then make it unit length.
            tangent -= Dot(normal[i], tangent) * normal[i];
            Normalize(tangent);

            // Compute the bitangent B, another tangent perpendicular to T.
            Vector3<float> bitangent = UnitCross(normal[i], tangent);

            // The set {T,B,N} is a right-handed orthonormal set.  The
            // negated light direction U = -D is represented in this
            // coordinate system as
            //   U = Dot(U,T)*T + Dot(U,B)*B + Dot(U,N)*N
            float dotUT = Dot(modelLightDirection, tangent);
            float dotUB = Dot(modelLightDirection, bitangent);
            float dotUN = Dot(modelLightDirection, normal[i]);

            // Transform the light vector into [0,1]^3 to make it a valid
            // Float3 object.
            currentDirection[0] = 0.5f * (dotUT + 1.0f);
            currentDirection[1] = 0.5f * (dotUB + 1.0f);
            currentDirection[2] = 0.5f * (dotUN + 1.0f);
        }
    }
}

bool SimpleBumpMapEffect::ComputeTangent(
    Vector3<float> const& position0, Vector2<float> const& tcoord0,
    Vector3<float> const& position1, Vector2<float> const& tcoord1,
    Vector3<float> const& position2, Vector2<float> const& tcoord2,
    Vector3<float>& tangent)
{
    // Compute the change in positions at the vertex P0.
    Vector3<float> deltaPos1 = position1 - position0;
    Vector3<float> deltaPos2 = position2 - position0;

    float const epsilon = 1e-08f;
    if (Length(deltaPos1) <= epsilon || Length(deltaPos1) <= epsilon )
    {
        // The triangle is degenerate.
        return false;
    }

    // Compute the change in texture coordinates at the vertex P0 in the
    // direction of edge P1-P0.
    float du1 = tcoord1[0] - tcoord0[0];
    float dv1 = tcoord1[1] - tcoord0[1];
    if (fabs(dv1) <= epsilon)
    {
        // The triangle effectively has no variation in the v texture
        // coordinate.
        if (fabs(du1) <= epsilon)
        {
            // The triangle effectively has no variation in the u coordinate.
            // Since the texture coordinates do not vary on this triangle,
            // treat it as a degenerate parametric surface.
            return false;
        }

        // The variation is effectively all in u, so set the tangent vector
        // to be T = dP/du.
        tangent = deltaPos1 / du1;
        return true;
    }

    // Compute the change in texture coordinates at the vertex P0 in the
    // direction of edge P2-P0.
    float du2 = tcoord2[0] - tcoord0[0];
    float dv2 = tcoord2[1] - tcoord0[1];
    float det = dv1 * du2 - dv2 * du1;
    if (fabs(det) <= epsilon)
    {
        // The triangle vertices are collinear in parameter space, so treat
        // this as a degenerate parametric surface.
        return false;
    }

    // The triangle vertices are not collinear in parameter space, so choose
    // the tangent to be dP/du = (dv1*dP2-dv2*dP1)/(dv1*du2-dv2*du1)
    tangent = (dv1 * deltaPos2 - dv2 * deltaPos1) / det;
    return true;
}

