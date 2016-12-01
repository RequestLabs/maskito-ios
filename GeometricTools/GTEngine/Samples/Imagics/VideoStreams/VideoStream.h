// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/DX11/GteDX11Engine.h>
#include <LowLevel/GteTimer.h>

class VideoStream
{
public:
    // The class is abstract.  Derived classes must implement the pure virtual
    // function 'char* GetImage ()'.
    virtual ~VideoStream();

    // Access to input information.
    inline gte::DX11Engine* GetEngine() const;
    inline gte::DFType GetType()const;
    inline unsigned int GetWidth() const;
    inline unsigned int GetHeight() const;

    // A frame consists of a frame number (unique identifier), the image data
    // represented as a texture, and the time (in microseconds) to acquire the
    // image and copy it to GPU memory.
    struct Frame
    {
        Frame() : number(0xFFFFFFFF), microseconds(0) {}
        unsigned int number;
        std::shared_ptr<gte::Texture2> image;
        int64_t microseconds;
    };

    // Get the current frame.  The returned frame is a copy of the member
    // data so that the texture may be consumed at any time without fear
    // of the producer overwriting the member data with a new frame.
    inline Frame GetFrame() const;

    // Support for production of a single frame.  The function assigns values
    // to the current frame.  It returns 'true' iff the production was
    // successful (a malformed file can lead to a failed read of data).
    void CaptureFrame();

    // Performance measurements.  These are accumulated measurements starting
    // from a call to ResetPerformanceMeasurements().
    void ResetPerformanceMeasurements();
    inline unsigned int GetPerformanceFrames() const;
    inline int64_t GetPerformanceMicroseconds() const;
    double GetFramesPerSecond() const;
    double GetSecondsPerFrame() const;

protected:
    // Constructor used by derived classes.
    VideoStream(gte::DX11Engine* engine);

    // A derived class must implement this function for its image capture
    // mechanism.  The function returns the image that is consumed in the
    // 'void CaptureFrame ()' call.
    virtual char* GetImage() = 0;

    // The engine that is used to upload textures to GPU memory and the
    // texture information.  The derived class must see these four members.
    gte::DX11Engine* mEngine;
    gte::DFType mType;
    unsigned int mWidth;
    unsigned int mHeight;

    // The current frame.  The timer is used to compute how long it takes to
    // produce the frame.
    Frame mFrame;
    gte::Timer mProductionTimer;

    // Performance measurements.
    gte::Timer mPerformanceTimer;
    unsigned int mPerformanceFrames;
    int64_t mPerformanceMicroseconds;
};

inline gte::DX11Engine* VideoStream::GetEngine() const
{
    return mEngine;
}

inline gte::DFType VideoStream::GetType() const
{
    return mType;
}

inline unsigned int VideoStream::GetWidth() const
{
    return mWidth;
}

inline unsigned int VideoStream::GetHeight() const
{
    return mHeight;
}

inline VideoStream::Frame VideoStream::GetFrame() const
{
    return mFrame;
}

inline unsigned int VideoStream::GetPerformanceFrames() const
{
    return mPerformanceFrames;
}

inline int64_t VideoStream::GetPerformanceMicroseconds() const
{
    return mPerformanceMicroseconds;
}
