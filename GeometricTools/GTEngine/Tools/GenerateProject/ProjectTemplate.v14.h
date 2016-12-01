// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <regex>
#include <string>

class TemplateV14
{
public:
    TemplateV14(std::string const& name, std::string const& gtPath, bool& success);

private:
    bool Create(std::string const& name, std::vector<std::string> const& lines, bool useUT8);
    static std::string GetGuidString();

    // Matching patterns.
    static std::regex const mGPPattern;
    static std::regex const mPNPattern;
    static std::regex const mPGPattern;
    static std::regex const mRQPattern;
    static std::regex const mGTPattern;

    // Replacements for patterns.
    std::string mName;
    std::string mGTPath;
    std::string mProjectGUID;
    std::string mRequiredGUID;
    static std::string const msGTGUID;

    enum
    {
        NUM_SOLUTION_LINES = 42,
        NUM_PROJECT_LINES = 179,
        NUM_FILTER_LINES = 26,
        NUM_APP_LINES = 59,
        NUM_WINH_LINES = 21,
        NUM_WINC_LINES = 25
    };

    // Templates for solution, project, project filter, and source code.
    static std::vector<std::string> const msSolutionLines;
    static std::vector<std::string> const msProjectLines;
    static std::vector<std::string> const msFilterLines;
    static std::vector<std::string> const msAppLines;
    static std::vector<std::string> const msWinHLines;
    static std::vector<std::string> const msWinCLines;
};
