// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include "FitSqrt.h"
#include "FitInvSqrt.h"
#include "FitSin.h"
#include "FitCos.h"
#include "FitTan.h"
#include "FitASin.h"
#include "FitATan.h"
#include "FitExp2.h"
#include "FitLog2.h"


int main(int, char const*[])
{
#if defined(_DEBUG)
    LogReporter reporter(
        "LogReport.txt",
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL,
        Logger::Listener::LISTEN_FOR_ALL);
#endif

    FitSqrt fitterSqrt;
    FitInvSqrt fitterInvSqrt;
    FitSin fitterSin;  // template parameter is 'order', degree = 2*order + 1
    FitCos fitterCos;  // template parameter is 'order', degree = 2*order
    FitTan fitterTan;  // template parameter is 'order', degree = 2*order + 1
    FitASin fitterASin;  // template parameter is 'order', degree = 2*order + 1
    FitATan fitterATan;  // template parameter is 'order', degree = 2*order + 1
    FitExp2 fitterExp2;
    FitLog2 fitterLog2;
    std::vector<double> poly;
    double error;
    fitterSin.Generate<6>(poly, error);
    return 0;
}

