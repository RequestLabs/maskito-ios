// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "BipedManager.h"
#include <Applications/GteWICFileIO.h>
#include <Graphics/GteBlendTransformController.h>
#include <Graphics/GteKeyframeController.h>
#include <LowLevel/GteLogger.h>
#include <fstream>


BipedManager::BipedManager(std::string const& rootPath,
    std::string const& bname, ProgramFactory& programFactory,
    SkinController::Updater const& postUpdate)
{
    // Vertex format shared by the two skins.
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_NORMAL, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    // Create the texture effects for the two skins.
    std::shared_ptr<Texture2> texture[2];
    std::shared_ptr<Texture2Effect> effect[2];
    for (int i = 0; i < 2; ++i)
    {
        std::string name = rootPath + "Skins/Skins" + std::to_string(i) + ".texture.png";
        texture[i].reset(WICFileIO::Load(name, true));
        texture[i]->AutogenerateMipmaps();
        effect[i] = std::make_shared<Texture2Effect>(programFactory,
            texture[i], SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::WRAP,
            SamplerState::WRAP);
    }

    PreSpatialArray preSpatialArray;
    PreSkinArray preSkinArray;
    SpatialMap spatialMap;

    std::string filename = rootPath + bname + ".txt";
    std::ifstream inFile(filename);
    while (!inFile.eof())
    {
        std::string line;
        getline(inFile, line);
        if (line == "")
        {
            // The file contains no blank lines, except for the very last one.
            break;
        }

        // Strip off initial white space.
        std::string::size_type begin = line.find_first_not_of(" ");
        if (begin > 0)
        {
            line = line.substr(begin);
        }

        std::string::size_type end;
        std::string name;
        if (line.find("Node") == 0)
        {
            // Load the node.
            begin = line.find("<");
            end = line.find(">");
            name = line.substr(begin + 1, end - 1 - begin);
            if (name.back() == 'X')
            {
                // TODO: These nodes are not necessary.  Remove them from the
                // data sets.
                continue;
            }

            PreSpatial* preSpatial = LoadNode(rootPath, name);
            preSpatialArray.push_back(preSpatial);
            Node* node = reinterpret_cast<Node*>(preSpatial->Associate);
            spatialMap[name] = node;

            // Load the transform controllers.
            NodeCtrl nc;
            nc.first = node;

            nc.second.reset(LoadTransformController(rootPath, name, "Idle"));
            mIdleArray.push_back(nc);

            nc.second.reset(LoadTransformController(rootPath, name, "Walk"));
            mWalkArray.push_back(nc);

            nc.second.reset(LoadTransformController(rootPath, name, "Run"));
            mRunArray.push_back(nc);
        }
        else if (line.find("TriMesh") == 0)
        {
            // Load the mesh.
            begin = line.find("<");
            end = line.find(">");
            name = line.substr(begin + 1, end - 1 - begin);
            int suffix = name[name.length() - 1] - '0';
            PreSpatial* preSpatial = LoadMesh(rootPath, name, vformat, effect[suffix]);
            preSpatialArray.push_back(preSpatial);
            spatialMap[name] = preSpatial->Associate;

            // Load the skin controller.
            PreSkin* preSkin = LoadSkinController(rootPath, name, postUpdate);
            preSkinArray.push_back(preSkin);

            // Attach the skin controller to the mesh.
            preSpatial->Associate->AttachController(preSkin->Associate);
        }
    }

    // Resolve the bone links.
    for (auto preSkin : preSkinArray)
    {
        SkinController* ctrl = preSkin->Associate;
        Node** bones = ctrl->GetBones();
        int i = 0;
        for (auto const& boneName : preSkin->BoneNames)
        {
            auto iter = spatialMap.find(boneName);
            bones[i] = reinterpret_cast<Node*>(iter->second);
            ++i;
        }
    }

    // Assemble the biped hierarchy.
    for (auto preSpatial : preSpatialArray)
    {
        Node* node = dynamic_cast<Node*>(preSpatial->Associate);
        if (node)
        {
            for (auto const& childName : preSpatial->ChildNames)
            {
                if (childName.back() == 'X')
                {
                    // TODO: These nodes are not necessary.  Remove them from
                    // the data sets.
                    continue;
                }

                auto iter = spatialMap.find(childName);
                Visual* mesh = dynamic_cast<Visual*>(iter->second);
                if (mesh)
                {
                    std::shared_ptr<Visual> visual(mesh);
                    node->AttachChild(visual);
                    if (visual->GetEffect().get() == effect[0].get())
                    {
                        mSubscribers[0].first = visual;
                        mSubscribers[0].second = effect[0]->GetPVWMatrixConstant();
                    }
                    else
                    {
                        mSubscribers[1].first = visual;
                        mSubscribers[1].second = effect[1]->GetPVWMatrixConstant();
                    }
                }
                else
                {
                    node->AttachChild(std::shared_ptr<Spatial>(iter->second));
                }
            }
        }
    }

    mRoot.reset(reinterpret_cast<Node*>(preSpatialArray[0]->Associate));

    for (auto preSpatial : preSpatialArray)
    {
        delete preSpatial;
    }

    for (auto preSkin : preSkinArray)
    {
        delete preSkin;
    }

    // Create the blend controllers.
    int const numControllers = static_cast<int>(mIdleArray.size());
    mIdleWalkArray.resize(numControllers);
    mWalkRunArray.resize(numControllers);
    for (int i = 0; i < numControllers; ++i)
    {
        NodeCtrl const& nc0 = mIdleArray[i];
        NodeCtrl const& nc1 = mWalkArray[i];
        NodeCtrl const& nc2 = mRunArray[i];

        mIdleWalkArray[i].first = nc0.first;
        mIdleWalkArray[i].second = std::make_shared<BlendTransformController>(
            nc0.second, nc1.second, true, false);

        mWalkRunArray[i].first = nc0.first;
        mWalkRunArray[i].second = std::make_shared<BlendTransformController>(
            nc1.second, nc2.second, true, false);
    }
}

void BipedManager::Initialize(int idleWalkCount, int walkCount,
    int walkRunCount)
{
    mState = ANIM_IDLE;
    mCount = 0;
    mCountMax[ANIM_IDLE] = 0;
    mCountMax[ANIM_IDLE_WALK] = idleWalkCount;
    mCountMax[ANIM_WALK] = mCountMax[ANIM_IDLE_WALK] + walkCount;
    mCountMax[ANIM_WALK_RUN] = mCountMax[ANIM_WALK] + walkRunCount;
    mCountMax[ANIM_RUN] = mCountMax[ANIM_WALK_RUN];
    mWeight = 0.0f;
    mDeltaWeight0 = 1.0f / static_cast<float>(idleWalkCount);
    mDeltaWeight1 = 1.0f / static_cast<float>(walkRunCount);

    DoIdle();
}

void BipedManager::Update(bool blendIdleToWalk, bool blendWalkToRun)
{
    if (blendIdleToWalk)
    {
        if (mState == ANIM_IDLE)
        {
            TransitionIdleToIdleWalk();
            mCount = 1;
            return;
        }

        if (mState == ANIM_IDLE_WALK)
        {
            if (mCount++ < mCountMax[ANIM_IDLE_WALK])
            {
                ContinueIdleWalk();
            }
            else
            {
                TransitionIdleWalkToWalk();
            }
            return;
        }

        if (mState == ANIM_WALK)
        {
            if (blendWalkToRun)
            {
                if (mCount++ == mCountMax[ANIM_WALK])
                {
                    TransitionWalkToWalkRun();
                }
            }
            else
            {
                // continue walk
            }
            return;
        }

        if (mState == ANIM_WALK_RUN)
        {
            if (blendWalkToRun)
            {
                if (mCount++ < mCountMax[ANIM_WALK_RUN])
                {
                    ContinueWalkRun();
                }
                else
                {
                    TransitionWalkRunToRun();
                }
            }
            else
            {
                if (--mCount > mCountMax[ANIM_WALK])
                {
                    ContinueRunWalk();
                }
                else
                {
                    TransitionRunWalkToWalk();
                }
            }
            return;
        }

        if (mState == ANIM_RUN)
        {
            if (blendWalkToRun)
            {
                // continue run
            }
            else
            {
                --mCount;
                TransitionRunToRunWalk();
            }
            return;
        }
    }
    else
    {
        if (mState == ANIM_RUN)
        {
            --mCount;
            TransitionRunToRunWalk();
            return;
        }

        if (mState == ANIM_WALK_RUN)
        {
            if (--mCount > mCountMax[ANIM_WALK])
            {
                ContinueRunWalk();
            }
            else
            {
                TransitionRunWalkToWalk();
            }
            return;
        }

        if (mState == ANIM_WALK)
        {
            if (--mCount == mCountMax[ANIM_IDLE_WALK])
            {
                TransitionWalkToWalkIdle();
            }
            else
            {
                // continue walk
            }
            return;
        }

        if (mState == ANIM_IDLE_WALK)
        {
            if (--mCount > mCountMax[ANIM_IDLE])
            {
                ContinueWalkIdle();
            }
            else
            {
                TransitionWalkIdleToIdle();
            }
            return;
        }

        if (mState == ANIM_IDLE)
        {
            // continue idle
            return;
        }
    }
}

float BipedManager::GetSpeed() const
{
    return
        static_cast<float>(mCount) / static_cast<float>(mCountMax[ANIM_RUN]);
}

BipedManager::PreSpatial* BipedManager::LoadNode(std::string const& rootPath,
    std::string const& name)
{
    PreSpatial* preSpatial = new PreSpatial();
    Node* node = new Node();
    node->name = name;
    preSpatial->Associate = node;

    std::string filename = rootPath + "Bones/" + name + ".node.raw";
    std::ifstream input(filename, std::ios::in | std::ios::binary);

    int numChildren;
    input.read((char*)&numChildren, sizeof(numChildren));
    for (int i = 0; i < numChildren; ++i)
    {
        int length;
        input.read((char*)&length, sizeof(length));
        char* text = new char[length + 1];
        input.read(text, sizeof(char) * length);
        text[length] = 0;
        preSpatial->ChildNames.push_back(std::string(text));
        delete[] text;
    }

    input.close();
    return preSpatial;
}

BipedManager::PreSpatial* BipedManager::LoadMesh(std::string const& rootPath,
    std::string const& name, VertexFormat const& vformat,
    std::shared_ptr<Texture2Effect> const& effect)
{
    std::string filename = rootPath + "Skins/" + name + ".mesh.raw";
    std::ifstream input(filename, std::ios::in | std::ios::binary);

    unsigned int numVertices;
    input.read((char*)&numVertices, sizeof(numVertices));
    std::shared_ptr<VertexBuffer> vbuffer = std::make_shared<VertexBuffer>(
        vformat, numVertices);
    input.read(vbuffer->GetData(), vbuffer->GetNumBytes());
    vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);

    int numIndices;
    input.read((char*)&numIndices, sizeof(numIndices));
    std::shared_ptr<IndexBuffer> ibuffer = std::make_shared<IndexBuffer>(
        IP_TRIMESH, numIndices / 3, sizeof(int));
    input.read(ibuffer->GetData(), numIndices * sizeof(int));
    input.close();

    Visual* mesh = new Visual(vbuffer, ibuffer, effect);
    mesh->name = name;

    PreSpatial* preSpatial = new PreSpatial();
    preSpatial->Associate = mesh;
    return preSpatial;
}

BipedManager::PreSkin* BipedManager::LoadSkinController(
    std::string const& rootPath, std::string const& name,
    SkinController::Updater const& postUpdate)
{
    std::string filename = rootPath + "Skins/" + name + ".skinctrl.raw";
    std::ifstream input(filename, std::ios::in | std::ios::binary);

    int repeatType, active;
    double minTime, maxTime, phase, frequency;
    input.read((char*)&repeatType, sizeof(repeatType));
    input.read((char*)&minTime, sizeof(minTime));
    input.read((char*)&maxTime, sizeof(maxTime));
    input.read((char*)&phase, sizeof(phase));
    input.read((char*)&frequency, sizeof(frequency));
    input.read((char*)&active, sizeof(active));

    int numVertices, numBones;
    input.read((char*)&numVertices, sizeof(numVertices));
    input.read((char*)&numBones, sizeof(numBones));

    SkinController* ctrl = new SkinController(numVertices, numBones, postUpdate);
    ctrl->name = name;
    PreSkin* preSkin = new PreSkin();
    preSkin->Associate = ctrl;

    ctrl->repeat = static_cast<Controller::RepeatType>(repeatType);
    ctrl->minTime = minTime;
    ctrl->maxTime = maxTime;
    ctrl->phase = phase;
    ctrl->frequency = frequency;
    ctrl->active = (active > 0);

    int b, v;
    for (b = 0; b < numBones; ++b)
    {
        int length;
        input.read((char*)&length, sizeof(length));
        char* text = new char[length + 1];
        input.read(text, sizeof(char) * length);
        text[length] = 0;
        preSkin->BoneNames.push_back(std::string(text));
        delete[] text;
    }

    float** weights = ctrl->GetWeights();
    Vector4<float>** offsets = ctrl->GetOffsets();

    for (v = 0; v < numVertices; ++v)
    {
        for (b = 0; b < numBones; ++b)
        {
            float weight;
            input.read((char*)&weight, sizeof(weight));
            weights[v][b] = weight;
        }
    }

    Vector4<float> offset{ 0.0f, 0.0f, 0.0f, 1.0f };
    for (v = 0; v < numVertices; ++v)
    {
        for (b = 0; b < numBones; ++b)
        {
            input.read((char*)&offset[0], 3 * sizeof(offset[0]));
            offsets[v][b] = offset;
        }
    }

    input.close();
    return preSkin;
}

TransformController* BipedManager::LoadTransformController(
    std::string const& rootPath, std::string const& name,
    std::string const& animation)
{
    std::string filename = rootPath + "Animations/" + animation + "/" +
        name + ".xfrmctrl.raw";
    std::ifstream input(filename, std::ios::in | std::ios::binary);

    int isKeyframeController;
    input.read((char*)&isKeyframeController, sizeof(isKeyframeController));

    TransformController* ctrl;
    int repeatType, active;
    double minTime, maxTime, phase, frequency;
    input.read((char*)&repeatType, sizeof(repeatType));
    input.read((char*)&minTime, sizeof(minTime));
    input.read((char*)&maxTime, sizeof(maxTime));
    input.read((char*)&phase, sizeof(phase));
    input.read((char*)&frequency, sizeof(frequency));
    input.read((char*)&active, sizeof(active));

    float mat[9], trn[3], sca[3];
    char isIdentity, isRSMatrix, isUniformScale, dummy;
    input.read((char*)mat, 9 * sizeof(float));
    input.read((char*)trn, 3 * sizeof(float));
    input.read((char*)sca, 3 * sizeof(float));
    input.read((char*)&isIdentity, sizeof(isIdentity));
    input.read((char*)&isRSMatrix, sizeof(isRSMatrix));
    input.read((char*)&isUniformScale, sizeof(isUniformScale));
    input.read((char*)&dummy, sizeof(dummy));

    Transform localTransform;
    localTransform.SetTranslation(trn[0], trn[1], trn[2]);

    if (isUniformScale)
    {
        localTransform.SetUniformScale(sca[0]);
    }
    else
    {
        localTransform.SetScale(sca[0], sca[1], sca[2]);
    }

#if defined(GTE_USE_MAT_VEC)
    if (isRSMatrix)
    {
        localTransform.SetRotation(Matrix4x4<float>({
            mat[0], mat[1], mat[2], 0.0f,
            mat[3], mat[4], mat[5], 0.0f,
            mat[6], mat[7], mat[8], 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f }));
    }
    else
    {
        localTransform.SetMatrix(Matrix4x4<float>({
            mat[0], mat[1], mat[2], 0.0f,
            mat[3], mat[4], mat[5], 0.0f,
            mat[6], mat[7], mat[8], 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f }));
    }
#else
    if (isRSMatrix)
    {
        localTransform.SetRotation(Matrix4x4<float>({
            mat[0], mat[3], mat[6], 0.0f,
            mat[1], mat[4], mat[7], 0.0f,
            mat[2], mat[5], mat[8], 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f }));
    }
    else
    {
        localTransform.SetMatrix(Matrix4x4<float>({
            mat[0], mat[3], mat[6], 0.0f,
            mat[1], mat[4], mat[7], 0.0f,
            mat[2], mat[5], mat[8], 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f }));
    }
#endif

    if (isKeyframeController)
    {
        int numTranslations, numRotations, numScales;
        input.read((char*)&numTranslations, sizeof(numTranslations));
        input.read((char*)&numRotations, sizeof(numRotations));
        input.read((char*)&numScales, sizeof(numScales));

        KeyframeController* keyfctrl = new KeyframeController(0,
            numTranslations, numRotations, numScales, localTransform);

        if (numTranslations > 0)
        {
            input.read((char*)keyfctrl->GetTranslationTimes(),
                sizeof(float) * numTranslations);

            input.read((char*)keyfctrl->GetTranslations(),
                sizeof(Vector4<float>) * numTranslations);
        }

        if (numRotations > 0)
        {
            input.read((char*)keyfctrl->GetRotationTimes(),
                sizeof(float) * numRotations);

            input.read((char*)keyfctrl->GetRotations(),
                sizeof(Quaternion<float>) * numRotations);
        }

        if (numScales > 0)
        {
            input.read((char*)keyfctrl->GetScaleTimes(),
                sizeof(float) * numScales);

            input.read((char*)keyfctrl->GetScales(),
                sizeof(float) * numScales);
        }

        ctrl = keyfctrl;
    }
    else
    {
        // The adjustment to the "Biped" root node is an attempt to get the
        // biped to idle/walk/run on a floor at height zero.  The biped model
        // should have minimally been created so that the foot steps occur at
        // the same height for all animations.
        if (name == "Biped")
        {
            Vector3<float> translate = localTransform.GetTranslation();
            if (animation == "Idle")
            {
                translate[2] += 3.5f;
            }
            else if (animation == "Walk")
            {
                translate[2] += 2.0f;
            }
            else // animation == "Run"
            {
                translate[2] += 2.5f;
            }
            localTransform.SetTranslation(translate);
        }

        TransformController* xfrmctrl =
            new TransformController(localTransform);

        ctrl = xfrmctrl;
    }

    ctrl->name = name;
    ctrl->repeat = Controller::RT_WRAP;
    ctrl->minTime = minTime;
    ctrl->maxTime = maxTime;
    ctrl->phase = phase;
    ctrl->frequency = frequency;
    ctrl->active = (active > 0);

    input.close();
    return ctrl;
}

void BipedManager::GetAnimation(NodeCtrlArray const& ncArray,
    double& minTime, double& maxTime) const
{
    minTime = std::numeric_limits<double>::max();
    maxTime = -minTime;
    for (auto const& nc : ncArray)
    {
        TransformController const* ctrl = nc.second.get();
        if (ctrl->minTime < minTime)
        {
            minTime = ctrl->minTime;
        }
        else if (ctrl->maxTime > maxTime)
        {
            maxTime = ctrl->maxTime;
        }
    }
}

void BipedManager::SetAnimation(NodeCtrlArray& ncArray, double frequency,
    double phase)
{
    for (auto& nc : ncArray)
    {
        TransformController* ctrl = nc.second.get();
        ctrl->frequency = frequency;
        ctrl->phase = phase;
    }
}

void BipedManager::SetBlendAnimation(NodeCtrlArray& ncArray, float weight)
{
    for (auto& nc : ncArray)
    {
        BlendTransformController* ctrl =
            reinterpret_cast<BlendTransformController*>(nc.second.get());
        ctrl->SetWeight(weight);
    }
}

void BipedManager::DoAnimation(NodeCtrlArray& ncArray)
{
    for (auto& nc : ncArray)
    {
        Node* node = nc.first;
        TransformController* ctrl = nc.second.get();
        ctrl->repeat = Controller::RT_WRAP;
        node->DetachAllControllers();
        node->AttachController(ctrl);
    }
}

void BipedManager::ContinueIdleWalk()
{
    SetIdleWalk(mWeight);
    mWeight += mDeltaWeight0;
    if (mWeight > 1.0f)
    {
        mWeight = 1.0f;
    }
}

void BipedManager::ContinueWalkRun()
{
    SetWalkRun(mWeight);
    mWeight += mDeltaWeight1;
    if (mWeight > 1.0f)
    {
        mWeight = 1.0f;
    }
}

void BipedManager::ContinueRunWalk()
{
    SetWalkRun(mWeight);
    mWeight -= mDeltaWeight1;
    if (mWeight < 0.0f)
    {
        mWeight = 0.0f;
    }
}

void BipedManager::ContinueWalkIdle()
{
    SetIdleWalk(mWeight);
    mWeight -= mDeltaWeight0;
    if (mWeight < 0.0f)
    {
        mWeight = 0.0f;
    }
}

void BipedManager::TransitionIdleToIdleWalk()
{
    mState = ANIM_IDLE_WALK;
    DoIdleWalk();
    SetIdleWalk(0.0f);
    mWeight = mDeltaWeight0;
}

void BipedManager::TransitionIdleWalkToWalk()
{
    mState = ANIM_WALK;
    DoWalk();
}

void BipedManager::TransitionWalkToWalkRun()
{
    mState = ANIM_WALK_RUN;
    DoWalkRun();
    SetWalkRun(0.0f);
    mWeight = mDeltaWeight1;
}

void BipedManager::TransitionWalkRunToRun()
{
    mState = ANIM_RUN;
    DoRun();
}

void BipedManager::TransitionRunToRunWalk()
{
    mState = ANIM_WALK_RUN;
    DoWalkRun();
    SetWalkRun(1.0f);
    mWeight = 1.0f - mDeltaWeight1;
}

void BipedManager::TransitionRunWalkToWalk()
{
    mState = ANIM_WALK;
    DoWalk();
}

void BipedManager::TransitionWalkToWalkIdle()
{
    mState = ANIM_IDLE_WALK;
    DoIdleWalk();
    SetIdleWalk(1.0f);
    mWeight = 1.0f - mDeltaWeight0;
}

void BipedManager::TransitionWalkIdleToIdle()
{
    mState = ANIM_IDLE;
    DoIdle();
}

