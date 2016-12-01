// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteNode.h>
#include <Graphics/GteVisual.h>
#include <Graphics/GteProgramFactory.h>
#include <Graphics/GteSkinController.h>
#include <Graphics/GteTexture2Effect.h>
#include <Graphics/GteTransformController.h>
#include <array>
#include <map>
#include <string>
using namespace gte;

class BipedManager
{
public:
    // Construction.
    BipedManager(std::string const& rootPath, std::string const& bname,
        ProgramFactory& programFactory,
        SkinController::Updater const& postUpdate);

    // Member access.
    inline std::shared_ptr<Node> const& GetRoot() const;

    // Pairs that need their transforms and pvw-buffers subscribed for
    // automatic updates.
    typedef std::pair<std::shared_ptr<Visual>,
        std::shared_ptr<ConstantBuffer >> Subscribers;
    inline std::array<Subscribers, 2> const& GetSubscribers() const;

    // Get the extreme times for all the controllers of the animation.
    inline void GetIdle(double& minTime, double& maxTime) const;
    inline void GetWalk(double& minTime, double& maxTime) const;
    inline void GetRun(double& minTime, double& maxTime) const;

    // Set time sampler parameters.
    inline void SetIdle(double frequency, double phase);
    inline void SetWalk(double frequency, double phase);
    inline void SetRun(double frequency, double phase);

    // Select animations.
    inline void DoIdle();
    inline void DoWalk();
    inline void DoRun();
    inline void DoIdleWalk();
    inline void DoWalkRun();

    // Set blending weight.
    inline void SetIdleWalk(float weight);
    inline void SetWalkRun(float weight);

    // Finite state machine for switching and blending among animations.

    // The input idleWalkCount is the maximum number of times Update samples
    // the blend of idle and walk before transitioning to idle or walk.  The
    // input walkCount is the maximum number of times Update samples the
    // walk when blendWalkToRun is 'true' in the Update function.  The input
    // walkRunCount is the maximum number of times Update samples the blend
    // of walk and run before transitioning to walk or run.
    void Initialize(int idleWalkCount, int walkCount, int walkRunCount);

    // Select and sample the appropriate animation.
    //
    // If blendIdleToWalk is 'false', the input blendWalkToRun is ignored.
    // The machine transitions from its current animation state (determined
    // by count) to the idle animation (count is decremented to zero).
    //
    // If blendIdleToWalk is 'true' and blendWalkToRun is 'false', then the
    // machine transitions from its current animation state (determined by
    // count) to the walk-animation state (count is incremented).  Once the
    // machine enters the walk-animation state, it stays in that state.
    //
    // If blendIdleToWalk is 'true' and blendWalkToRun is 'true', then the
    // machine transitions from its current animation state (determined by
    // count) to the run-animation state (count is incremented).  Once the
    // machine enters the run-animation state, it stays in that state.
    void Update(bool blendIdleToWalk, bool blendWalkToRun);

    // Get the speed of the biped.  This depends on the current animation
    // state.  The speed here is dimensionless, measured as the ratio
    // mCount/mCountMax[ANIM_RUN], which is in [0,1].
    float GetSpeed() const;

private:
    // Loading support.
    typedef std::vector<std::string> StringArray;
    typedef std::map<std::string, Spatial*> SpatialMap;
    typedef std::pair<Node*, std::shared_ptr<TransformController>> NodeCtrl;
    typedef std::vector<NodeCtrl> NodeCtrlArray;

    class PreSpatial
    {
    public:
        Spatial* Associate;
        StringArray ChildNames;
    };
    typedef std::vector<PreSpatial*> PreSpatialArray;

    class PreSkin
    {
    public:
        SkinController* Associate;
        StringArray BoneNames;
    };
    typedef std::vector<PreSkin*> PreSkinArray;

    PreSpatial* LoadNode(std::string const& rootPath,
        std::string const& name);

    PreSpatial* LoadMesh(std::string const& rootPath,
        std::string const& name, VertexFormat const& vformat,
        std::shared_ptr<Texture2Effect> const& effect);

    PreSkin* LoadSkinController(std::string const& rootPath,
        std::string const& name, SkinController::Updater const& postUpdate);

    TransformController* LoadTransformController(
        std::string const& rootPath, std::string const& name,
        std::string const& animation);

    // Run-time support.
    void GetAnimation(NodeCtrlArray const& ncArray, double& minTime,
        double& maxTime) const;
    void SetAnimation(NodeCtrlArray& ncArray, double frequency,
        double phase);
    void SetBlendAnimation(NodeCtrlArray& ncArray, float weight);
    void DoAnimation(NodeCtrlArray& ncArray);

    // The biped and animation sequences.
    std::shared_ptr<Node> mRoot;
    std::array<Subscribers, 2> mSubscribers;
    NodeCtrlArray mIdleArray, mWalkArray, mRunArray;
    NodeCtrlArray mIdleWalkArray, mWalkRunArray;

    // Finite state machine.
    enum Animation
    {
        ANIM_IDLE,
        ANIM_IDLE_WALK,
        ANIM_WALK,
        ANIM_WALK_RUN,
        ANIM_RUN,
        NUM_STATES
    };

    void ContinueIdleWalk();
    void ContinueWalkRun();
    void ContinueRunWalk();
    void ContinueWalkIdle();
    void TransitionIdleToIdleWalk();
    void TransitionIdleWalkToWalk();
    void TransitionWalkToWalkRun();
    void TransitionWalkRunToRun();
    void TransitionRunToRunWalk();
    void TransitionRunWalkToWalk();
    void TransitionWalkToWalkIdle();
    void TransitionWalkIdleToIdle();

    Animation mState;
    int mCount, mCountMax[NUM_STATES];
    float mWeight, mDeltaWeight0, mDeltaWeight1;
};


inline std::shared_ptr<Node> const& BipedManager::GetRoot() const
{
    return mRoot;
}

inline std::array<BipedManager::Subscribers, 2> const&
BipedManager::GetSubscribers() const
{
    return mSubscribers;
}

inline void BipedManager::GetIdle(double& minTime, double& maxTime) const
{
    GetAnimation(mIdleArray, minTime, maxTime);
}

inline void BipedManager::GetWalk(double& minTime, double& maxTime) const
{
    GetAnimation(mWalkArray, minTime, maxTime);
}

inline void BipedManager::GetRun(double& minTime, double& maxTime) const
{
    GetAnimation(mRunArray, minTime, maxTime);
}

inline void BipedManager::SetIdle(double frequency, double phase)
{
    SetAnimation(mIdleArray, frequency, phase);
}

inline void BipedManager::SetWalk(double frequency, double phase)
{
    SetAnimation(mWalkArray, frequency, phase);
}

inline void BipedManager::SetRun(double frequency, double phase)
{
    SetAnimation(mRunArray, frequency, phase);
}

inline void BipedManager::DoIdle()
{
    DoAnimation(mIdleArray);
}

inline void BipedManager::DoWalk()
{
    DoAnimation(mWalkArray);
}

inline void BipedManager::DoRun()
{
    DoAnimation(mRunArray);
}

inline void BipedManager::DoIdleWalk()
{
    DoAnimation(mIdleWalkArray);
}

inline void BipedManager::DoWalkRun()
{
    DoAnimation(mWalkRunArray);
}

inline void BipedManager::SetIdleWalk(float weight)
{
    SetBlendAnimation(mIdleWalkArray, weight);
}

inline void BipedManager::SetWalkRun(float weight)
{
    SetBlendAnimation(mWalkRunArray, weight);
}

