// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteCamera.h>
#include <Graphics/GteConstantBuffer.h>
#include <map>

namespace gte
{

class GTE_IMPEXP CameraRig
{
public:
    // Construction.  The camera rig is designed for moving the camera around
    // in a world coordinate system and updating the pvw-matrices of any
    // object (via its constant buffers) that wants to listen for camera
    // position and orientation changes.  This object is used in the sample
    // 3D applications.
    virtual ~CameraRig();
    CameraRig();
    CameraRig(std::shared_ptr<Camera> const& camera, float translationSpeed,
        float rotationSpeed, BufferUpdater const& updater);

    // Member access.  The functions are for deferred construction after
    // a default construction of a camera rig.
    void Set(std::shared_ptr<Camera> const& camera, float translationSpeed,
        float rotationSpeed, BufferUpdater const& updater);

    // TODO:  The camera vectors are the world axes?  (INVARIANT)
    void ComputeWorldAxes();

    inline std::shared_ptr<Camera> const& GetCamera() const;
    inline void SetTranslationSpeed(float translationSpeed);
    inline float GetTranslationSpeed() const;
    inline void SetRotationSpeed(float rotationSpeed);
    inline float GetRotationSpeed() const;
    inline void SetUpdater(BufferUpdater const& updater);
    inline BufferUpdater const& GetUpdater() const;

    // Control of camera motion.  If the camera moves, subscribers to the
    // pvw-matrix update will have the system memory and GPU memory of the
    // constant buffers updated.  Only one motion may be active at a single
    // time.  When a motion is active, a call to Move() will execute that
    // motion.  To disable camera motion, call DisableMotion().

    // The motion is controlled directly by calling SetDirect*().
    inline void SetDirectMoveForward();
    inline void SetDirectMoveBackward();
    inline void SetDirectMoveUp();
    inline void SetDirectMoveDown();
    inline void SetDirectMoveRight();
    inline void SetDirectMoveLeft();
    inline void SetDirectTurnRight();
    inline void SetDirectTurnLeft();
    inline void SetDirectLookUp();
    inline void SetDirectLookDown();
    inline void SetDirectRollClockwise();
    inline void SetDirectRollCounterclockwise();

    // The motion is controlled indirectly.  TheRegister* calls map the
    // 'trigger' to the function specified by the *-suffix.  If trigger >= 0,
    // the function is added to a map.  If trigger < 0 in the Register*
    // function, the corresponding function is removed from the map.  A call
    // to SetIndirectMotion(trigger) will set the active motion if the trigger
    // is currently mapped.  The Boolean return of SetIndirectMotion is 'true'
    // iff the trigger is mapped.
    inline void RegisterMoveForward(int trigger);
    inline void RegisterMoveBackward(int trigger);
    inline void RegisterMoveUp(int trigger);
    inline void RegisterMoveDown(int trigger);
    inline void RegisterMoveRight(int trigger);
    inline void RegisterMoveLeft(int trigger);
    inline void RegisterTurnRight(int trigger);
    inline void RegisterTurnLeft(int trigger);
    inline void RegisterLookUp(int trigger);
    inline void RegisterLookDown(int trigger);
    inline void RegisterRollClockwise(int trigger);
    inline void RegisterRollCounterclockwise(int trigger);
    bool PushMotion(int trigger);
    bool PopMotion(int trigger);

    bool Move();
    void ClearMotions();

    // Update the constant buffer's projection-view-world matrix (pvw-matrix)
    // when the camera's view or projection matrices change.  The input
    // 'pvwMatrixName' is the name specified in the shader program and is
    // used in calls to ConstantBuffer::SetMember<Matrix4x4<float>>(...).
    // If you modify the view or projection matrices directly through the
    // Camera interface, you are responsible for calling UpdatePVWMatrices().
    //
    // The SubscribeToPVWUpdate uses the address of 'worldMatrix' as a key
    // to a std::map, so be careful to ensure that 'worldMatrix' persists
    // until a call to an Unsubscribe* function.  The return value of
    // SubscribeToPVWUpdate is 'true' as long as 'cbuffer' is not already
    // subscribed and actually has a member named 'pvwMatrixName'.  The
    // return value of Unsubscribe(...) is true if and only if the input
    // matrix is currently subscribed.
    bool Subscribe(Matrix4x4<float> const& worldMatrix,
        std::shared_ptr<ConstantBuffer> const& cbuffer,
        std::string const& pvwMatrixName = "pvwMatrix");
    bool Unsubscribe(Matrix4x4<float> const& worldMatrix);
    void UnsubscribeAll();

    // After any camera modifictions that change the projection or view
    // matrices, call this function to update the constant buffers that
    // are subscribed.
    void UpdatePVWMatrices();

protected:
    // Camera motion.  These are called based on the state set in the
    // public interface.
    enum { MAX_ACTIVE_MOTIONS = 12 };
    virtual void MoveForward();
    virtual void MoveBackward();
    virtual void MoveUp();
    virtual void MoveDown();
    virtual void MoveRight();
    virtual void MoveLeft();
    virtual void TurnRight();
    virtual void TurnLeft();
    virtual void LookUp();
    virtual void LookDown();
    virtual void RollClockwise();
    virtual void RollCounterclockwise();

    typedef void(CameraRig::*MoveFunction)(void);
    void Register(int trigger, MoveFunction function);

    bool SetActive(MoveFunction function);
    bool SetInactive(MoveFunction function);

    std::shared_ptr<Camera> mCamera;
    float mTranslationSpeed, mRotationSpeed;
    BufferUpdater mUpdater;

    // Maintain the world coordinate system.
    Vector4<float> mWorldAxis[3];

    // Move via direct or indirect triggers.
    MoveFunction mMotion;
    std::map<int, MoveFunction> mIndirectMap;
    int mNumActiveMotions;
    std::array<MoveFunction, MAX_ACTIVE_MOTIONS> mActiveMotions;

    // Subscribers to camera matrix changes.
    typedef Matrix4x4<float> const* PVWKey;
    typedef std::pair<std::shared_ptr<ConstantBuffer>, std::string> PVWValue;
    std::map<PVWKey, PVWValue> mSubscribers;
};


inline std::shared_ptr<Camera> const& CameraRig::GetCamera() const
{
    return mCamera;
}

inline void CameraRig::SetTranslationSpeed(float translationSpeed)
{
    mTranslationSpeed = translationSpeed;
}

inline float CameraRig::GetTranslationSpeed() const
{
    return mTranslationSpeed;
}

inline void CameraRig::SetRotationSpeed(float rotationSpeed)
{
    mRotationSpeed = rotationSpeed;
}

inline float CameraRig::GetRotationSpeed() const
{
    return mRotationSpeed;
}

inline void CameraRig::SetUpdater(BufferUpdater const& updater)
{
    mUpdater = updater;
}

inline BufferUpdater const& CameraRig::GetUpdater() const
{
    return mUpdater;
}

inline void CameraRig::SetDirectMoveForward()
{
    mMotion = &CameraRig::MoveForward;
}

inline void CameraRig::SetDirectMoveBackward()
{
    mMotion = &CameraRig::MoveBackward;
}

inline void CameraRig::SetDirectMoveUp()
{
    mMotion = &CameraRig::MoveUp;
}

inline void CameraRig::SetDirectMoveDown()
{
    mMotion = &CameraRig::MoveDown;
}

inline void CameraRig::SetDirectMoveRight()
{
    mMotion = &CameraRig::MoveRight;
}

inline void CameraRig::SetDirectMoveLeft()
{
    mMotion = &CameraRig::MoveLeft;
}

inline void CameraRig::SetDirectTurnRight()
{
    mMotion = &CameraRig::TurnRight;
}

inline void CameraRig::SetDirectTurnLeft()
{
    mMotion = &CameraRig::TurnLeft;
}

inline void CameraRig::SetDirectLookUp()
{
    mMotion = &CameraRig::LookUp;
}

inline void CameraRig::SetDirectLookDown()
{
    mMotion = &CameraRig::LookDown;
}

inline void CameraRig::SetDirectRollClockwise()
{
    mMotion = &CameraRig::RollClockwise;
}

inline void CameraRig::SetDirectRollCounterclockwise()
{
    mMotion = &CameraRig::RollCounterclockwise;
}

inline void CameraRig::RegisterMoveForward(int trigger)
{
    Register(trigger, &CameraRig::MoveForward);
}

inline void CameraRig::RegisterMoveBackward(int trigger)
{
    Register(trigger, &CameraRig::MoveBackward);
}

inline void CameraRig::RegisterMoveUp(int trigger)
{
    Register(trigger, &CameraRig::MoveUp);
}

inline void CameraRig::RegisterMoveDown(int trigger)
{
    Register(trigger, &CameraRig::MoveDown);
}

inline void CameraRig::RegisterMoveRight(int trigger)
{
    Register(trigger, &CameraRig::MoveRight);
}

inline void CameraRig::RegisterMoveLeft(int trigger)
{
    Register(trigger, &CameraRig::MoveLeft);
}

inline void CameraRig::RegisterTurnRight(int trigger)
{
    Register(trigger, &CameraRig::TurnRight);
}

inline void CameraRig::RegisterTurnLeft(int trigger)
{
    Register(trigger, &CameraRig::TurnLeft);
}

inline void CameraRig::RegisterLookUp(int trigger)
{
    Register(trigger, &CameraRig::LookUp);
}

inline void CameraRig::RegisterLookDown(int trigger)
{
    Register(trigger, &CameraRig::LookDown);
}

inline void CameraRig::RegisterRollClockwise(int trigger)
{
    Register(trigger, &CameraRig::RollClockwise);
}

inline void CameraRig::RegisterRollCounterclockwise(int trigger)
{
    Register(trigger, &CameraRig::RollCounterclockwise);
}


}
