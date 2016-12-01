// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Graphics/GteCameraRig.h>
#include <Graphics/DX11/GteDX11Engine.h>
#if defined(GTE_DEV_OPENGL)
#include <Graphics/GL4/GteGL4Engine.h>
#endif
#include <Mathematics/GteRotation.h>
#include <algorithm>
using namespace gte;

CameraRig::~CameraRig()
{
}

CameraRig::CameraRig()
{
    Set(nullptr, 0.0f, 0.0f, [](std::shared_ptr<Buffer> const&){});
}

CameraRig::CameraRig(std::shared_ptr<Camera> const& camera,
    float translationSpeed, float rotationSpeed, BufferUpdater const& updater)
{
    Set(camera, translationSpeed, rotationSpeed, updater);
}

void CameraRig::Set(std::shared_ptr<Camera> const& camera,
    float translationSpeed, float rotationSpeed, BufferUpdater const& updater)
{
    mCamera = camera;
    mTranslationSpeed = translationSpeed;
    mRotationSpeed = rotationSpeed;
    mUpdater = updater;
    ComputeWorldAxes();
    ClearMotions();
}

void CameraRig::ComputeWorldAxes()
{
    if (mCamera)
    {
        mWorldAxis[0] = mCamera->GetDVector();
        mWorldAxis[1] = mCamera->GetUVector();
        mWorldAxis[2] = mCamera->GetRVector();
    }
    else
    {
        mWorldAxis[0].MakeZero();
        mWorldAxis[1].MakeZero();
        mWorldAxis[2].MakeZero();
    }
}

bool CameraRig::PushMotion(int trigger)
{
    auto element = mIndirectMap.find(trigger);
    return (element != mIndirectMap.end() ? SetActive(element->second) : false);
}

bool CameraRig::PopMotion(int trigger)
{
    auto element = mIndirectMap.find(trigger);
    return (element != mIndirectMap.end() ? SetInactive(element->second) : false);
}

bool CameraRig::Move()
{
    if (mMotion)
    {
        (this->*mMotion)();
        UpdatePVWMatrices();
        return true;
    }
    return false;
}

void CameraRig::ClearMotions()
{
    mMotion = nullptr;
    mIndirectMap.clear();
    mNumActiveMotions = 0;
    std::fill(mActiveMotions.begin(), mActiveMotions.end(), nullptr);
}

void CameraRig::MoveForward()
{
    if (mCamera)
    {
        mCamera->SetPosition(mCamera->GetPosition() + mTranslationSpeed * mWorldAxis[0]);
    }
}

void CameraRig::MoveBackward()
{
    if (mCamera)
    {
        mCamera->SetPosition(mCamera->GetPosition() - mTranslationSpeed * mWorldAxis[0]);
    }
}

void CameraRig::MoveUp()
{
    if (mCamera)
    {
        mCamera->SetPosition(mCamera->GetPosition() + mTranslationSpeed*mWorldAxis[1]);
    }
}

void CameraRig::MoveDown()
{
    if (mCamera)
    {
        mCamera->SetPosition(mCamera->GetPosition() - mTranslationSpeed*mWorldAxis[1]);
    }
}

void CameraRig::MoveRight()
{
    if (mCamera)
    {
        mCamera->SetPosition(mCamera->GetPosition() + mTranslationSpeed*mWorldAxis[2]);
    }
}

void CameraRig::MoveLeft()
{
    if (mCamera)
    {
        mCamera->SetPosition(mCamera->GetPosition() - mTranslationSpeed*mWorldAxis[2]);
    }
}

void CameraRig::TurnRight()
{
    if (mCamera)
    {
        Matrix4x4<float> incremental = Rotation<4, float>(
            AxisAngle<4, float>(mWorldAxis[1], -mRotationSpeed));

#if defined(GTE_USE_MAT_VEC)
        mWorldAxis[0] = incremental * mWorldAxis[0];
        mWorldAxis[2] = incremental * mWorldAxis[2];
        mCamera->SetAxes(
            incremental * mCamera->GetDVector(),
            incremental * mCamera->GetUVector(),
            incremental * mCamera->GetRVector());
#else
        mWorldAxis[0] = mWorldAxis[0] * incremental;
        mWorldAxis[2] = mWorldAxis[2] * incremental;
        mCamera->SetAxes(
            mCamera->GetDVector() * incremental,
            mCamera->GetUVector() * incremental,
            mCamera->GetRVector() * incremental);
#endif
    }
}

void CameraRig::TurnLeft()
{
    if (mCamera)
    {
        Matrix4x4<float> incremental = Rotation<4, float>(
            AxisAngle<4, float>(mWorldAxis[1], +mRotationSpeed));

#if defined(GTE_USE_MAT_VEC)
        mWorldAxis[0] = incremental * mWorldAxis[0];
        mWorldAxis[2] = incremental * mWorldAxis[2];
        mCamera->SetAxes(
            incremental*mCamera->GetDVector(),
            incremental*mCamera->GetUVector(),
            incremental*mCamera->GetRVector());
#else
        mWorldAxis[0] = mWorldAxis[0] * incremental;
        mWorldAxis[2] = mWorldAxis[2] * incremental;
        mCamera->SetAxes(
            mCamera->GetDVector() * incremental,
            mCamera->GetUVector() * incremental,
            mCamera->GetRVector() * incremental);
#endif
    }
}

void CameraRig::LookUp()
{
    if (mCamera)
    {
        Matrix4x4<float> incremental = Rotation<4, float>(
            AxisAngle<4, float>(mWorldAxis[2], +mRotationSpeed));

#if defined(GTE_USE_MAT_VEC)
        mCamera->SetAxes(
            incremental * mCamera->GetDVector(),
            incremental * mCamera->GetUVector(),
            incremental * mCamera->GetRVector());
#else
        mCamera->SetAxes(
            mCamera->GetDVector() * incremental,
            mCamera->GetUVector() * incremental,
            mCamera->GetRVector() * incremental);
#endif
    }
}

void CameraRig::LookDown()
{
    if (mCamera)
    {
        Matrix4x4<float> incremental = Rotation<4, float>(
            AxisAngle<4, float>(mWorldAxis[2], -mRotationSpeed));

#if defined(GTE_USE_MAT_VEC)
        mCamera->SetAxes(
            incremental * mCamera->GetDVector(),
            incremental * mCamera->GetUVector(),
            incremental * mCamera->GetRVector());
#else
        mCamera->SetAxes(
            mCamera->GetDVector() * incremental,
            mCamera->GetUVector() * incremental,
            mCamera->GetRVector() * incremental);
#endif
    }
}

void CameraRig::RollClockwise()
{
    if (mCamera)
    {
        Matrix4x4<float> incremental = Rotation<4, float>(
            AxisAngle<4, float>(mWorldAxis[0], +mRotationSpeed));

#if defined(GTE_USE_MAT_VEC)
        mCamera->SetAxes(
            incremental * mCamera->GetDVector(),
            incremental * mCamera->GetUVector(),
            incremental * mCamera->GetRVector());
#else
        mCamera->SetAxes(
            mCamera->GetDVector() * incremental,
            mCamera->GetUVector() * incremental,
            mCamera->GetRVector() * incremental);
#endif
    }
}

void CameraRig::RollCounterclockwise()
{
    if (mCamera)
    {
        Matrix4x4<float> incremental = Rotation<4, float>(
            AxisAngle<4, float>(mWorldAxis[0], -mRotationSpeed));

#if defined(GTE_USE_MAT_VEC)
        mCamera->SetAxes(
            incremental * mCamera->GetDVector(),
            incremental * mCamera->GetUVector(),
            incremental * mCamera->GetRVector());
#else
        mCamera->SetAxes(
            mCamera->GetDVector() * incremental,
            mCamera->GetUVector() * incremental,
            mCamera->GetRVector() * incremental);
#endif
    }
}

void CameraRig::Register(int trigger, MoveFunction function)
{
    if (trigger >= 0)
    {
        auto element = mIndirectMap.find(trigger);
        if (element == mIndirectMap.end())
        {
            mIndirectMap.insert(std::make_pair(trigger, function));
        }
    }
    else
    {
        for (auto element : mIndirectMap)
        {
            if (element.second == function)
            {
                mIndirectMap.erase(trigger);
                return;
            }
        }
    }
}

bool CameraRig::SetActive(MoveFunction function)
{
    for (int i = 0; i < mNumActiveMotions; ++i)
    {
        if (mActiveMotions[i] == function)
        {
            return false;
        }
    }

    if (mNumActiveMotions < MAX_ACTIVE_MOTIONS)
    {
        mMotion = function;
        mActiveMotions[mNumActiveMotions] = function;
        ++mNumActiveMotions;
        return true;
    }

    return false;
}

bool CameraRig::SetInactive(MoveFunction function)
{
    for (int i = 0; i < mNumActiveMotions; ++i)
    {
        if (mActiveMotions[i] == function)
        {
            for (int j0 = i, j1 = j0 + 1; j1 < mNumActiveMotions; j0 = j1++)
            {
                mActiveMotions[j0] = mActiveMotions[j1];
            }
            --mNumActiveMotions;
            mActiveMotions[mNumActiveMotions] = nullptr;
            if (mNumActiveMotions > 0)
            {
                mMotion = mActiveMotions[mNumActiveMotions - 1];
            }
            else
            {
                mMotion = nullptr;
            }
            return true;
        }
    }
    return false;
}

bool CameraRig::Subscribe(Matrix4x4<float> const& worldMatrix,
    std::shared_ptr<ConstantBuffer> const& cbuffer,
    std::string const& pvwMatrixName)
{
    if (cbuffer && cbuffer->HasMember(pvwMatrixName))
    {
        if (mSubscribers.find(&worldMatrix) == mSubscribers.end())
        {
            mSubscribers.insert(std::make_pair(&worldMatrix,
                std::make_pair(cbuffer, pvwMatrixName)));
            return true;
        }
    }
    return false;
}

bool CameraRig::Unsubscribe(Matrix4x4<float> const& worldMatrix)
{
    return mSubscribers.erase(&worldMatrix) > 0;
}

void CameraRig::UnsubscribeAll()
{
    mSubscribers.clear();
}

void CameraRig::UpdatePVWMatrices()
{
    // The function is called knowing that mCamera is not null.
    Matrix4x4<float> pvMatrix = mCamera->GetProjectionViewMatrix();
    for (auto& element : mSubscribers)
    {
        // Compute the new projection-view-world matrix.  The matrix
        // *element.first is the model-to-world matrix for the associated
        // object.
#if defined(GTE_USE_MAT_VEC)
        Matrix4x4<float> pvwMatrix = pvMatrix * (*element.first);
#else
        Matrix4x4<float> pvwMatrix = (*element.first) * pvMatrix;
#endif
        // Copy the source matrix into the system memory of the constant
        // buffer.
        element.second.first->SetMember(element.second.second, pvwMatrix);

        // Allow the caller to update GPU memory as desired.
        mUpdater(element.second.first);
    }
}
