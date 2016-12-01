// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngine.h>
#include "VideoStreamManager.h"
#include "FileVideoStream.h"
using namespace gte;

// NOTE: Expose only one of these.
//#define DO_MANUAL_SERIAL
//#define DO_MANUAL_PARALLEL
#define DO_TRIGGERED_SERIAL
//#define DO_TRIGGERED_PARALLEL

class VideoStreamsWindow : public Window
{
public:
    virtual ~VideoStreamsWindow();
    VideoStreamsWindow(Parameters& parameters);

    virtual void OnIdle();
    virtual bool OnCharPress(unsigned char key, int x, int y);

private:
    bool CreateOverlays(int textureWidth, int textureHeight);
    void DrawStatistics();

    enum { NUM_VIDEO_STREAMS = 4 };
    std::vector<VideoStream*> mVideoStreams;
    std::vector<std::shared_ptr<OverlayEffect>> mOverlay;
    VideoStreamManager* mVideoStreamManager;
    VideoStreamManager::Frame mCurrent;
};
