// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteController.h>
#include <Graphics/GteVertexBuffer.h>
#include <Mathematics/GteVector4.h>
#include <functional>
#include <memory>

namespace gte
{

class Node;

class GTE_IMPEXP SkinController : public Controller
{
public:
    // Construction and destruction.  The numbers of vertices and bones are
    // fixed for the lifetime of the object.  The controlled object must
    // have a vertex buffer with 'numVertices' elements, with 3D (x,y,z) or
    // 4D (x,y,z,1) positions, and the bind of positions is in unit 0.  The
    // post-update function is used to allow a graphics engine object to copy
    // the modified vertex buffer to graphics memory.
    typedef std::function<void(std::shared_ptr<VertexBuffer> const&)> Updater;
    virtual ~SkinController();
    SkinController(int numVertices, int numBones, Updater const& postUpdate);

    // Member access.  After calling the constructor, you must set the data
    // using these functions.
    inline int GetNumVertices() const;
    inline int GetNumBones() const;
    inline Node** GetBones() const;
    inline float** GetWeights() const;
    inline Vector4<float>** GetOffsets() const;

    // The animation update.  The application time is in milliseconds.
    virtual bool Update(double applicationTime);

protected:
    // On the first call to Update(...), the position channel and stride
    // are extracted from mObject's vertex buffer.  This is a deferred
    // construction, because we do not know mObject when SkinController
    // is constructed.
    void OnFirstUpdate();

    int mNumVertices;   // nv
    int mNumBones;      // nb
    Node** mBones;      // bones[nb]
    float** mWeights;   // weight[nv][nb], index b+nb*v
    Vector4<float>** mOffsets;  // offset[nv][nb], index b+nb*v
    Updater mPostUpdate;
    char* mPosition;
    unsigned int mStride;
    bool mFirstUpdate, mCanUpdate;
};


inline int SkinController::GetNumVertices() const
{
    return mNumVertices;
}

inline int SkinController::GetNumBones() const
{
    return mNumBones;
}

inline Node** SkinController::GetBones() const
{
    return mBones;
}

inline float** SkinController::GetWeights() const
{
    return mWeights;
}

inline Vector4<float>** SkinController::GetOffsets() const
{
    return mOffsets;
}


}
