// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <LowLevel/GteLogReporter.h>
using namespace gte;


LogReporter::~LogReporter()
{
    if (mLogToStdout)
    {
        Logger::Unsubscribe(mLogToStdout);
        delete mLogToStdout;
    }

    if (mLogToOutputWindow)
    {
        Logger::Unsubscribe(mLogToOutputWindow);
        delete mLogToOutputWindow;
    }

    if (mLogToMessageBox)
    {
        Logger::Unsubscribe(mLogToMessageBox);
        delete mLogToMessageBox;
    }

    if (mLogToFile)
    {
        Logger::Unsubscribe(mLogToFile);
        delete mLogToFile;
    }
}

LogReporter::LogReporter(std::string const& logFile, int logFileFlags,
    int logMessageBoxFlags, int logOutputWindowFlags, int logStdoutFlags)
    :
    mLogToFile(nullptr),
    mLogToMessageBox(nullptr),
    mLogToOutputWindow(nullptr),
    mLogToStdout(nullptr)
{
    if (logFileFlags != Logger::Listener::LISTEN_FOR_NOTHING)
    {
        mLogToFile = new LogToFile(logFile, logFileFlags);
        Logger::Subscribe(mLogToFile);
    }

    if (logMessageBoxFlags != Logger::Listener::LISTEN_FOR_NOTHING)
    {
        mLogToMessageBox = new LogToMessageBox(logMessageBoxFlags);
        Logger::Subscribe(mLogToMessageBox);
    }

    if (logOutputWindowFlags != Logger::Listener::LISTEN_FOR_NOTHING)
    {
        mLogToOutputWindow = new LogToOutputWindow(logOutputWindowFlags);
        Logger::Subscribe(mLogToOutputWindow);
    }

    if (logStdoutFlags != Logger::Listener::LISTEN_FOR_NOTHING)
    {
        mLogToStdout = new LogToStdout(logStdoutFlags);
        Logger::Subscribe(mLogToStdout);
    }
}

