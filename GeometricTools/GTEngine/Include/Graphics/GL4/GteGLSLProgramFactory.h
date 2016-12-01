// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteProgramFactory.h>
#include <Graphics/GL4/GteOpenGL.h>

namespace gte
{

class GTE_IMPEXP GLSLProgramFactory : public ProgramFactory
{
public:
    // The 'defaultVersion' can be set once on application initialization if
    // you want a GLSL version different from our default when constructing a
    // program factory.
    static std::string defaultVersion;  // "#version 430"
    static std::string defaultVSEntry;  // "main"
    static std::string defaultPSEntry;  // "main"
    static std::string defaultGSEntry;  // "main"
    static std::string defaultCSEntry;  // "main"
    static unsigned int defaultFlags;   // 0

    // Construction and destruction.  The 'version' member is set to
    // 'defaultVersion'.  The 'defines' are empty.
    virtual ~GLSLProgramFactory();
    GLSLProgramFactory();

    // The returned value is used as a lookup index into arrays of strings
    // corresponding to shader programs.
    virtual int GetAPI() const;

    // GLSLVisualProgram and GLSLComputeProgram objects are responsible
    // for destroying the shaders and program.  The factory wraps the
    // program objects as shared pointers to allow automatic clean-up.

private:

    // Create a program for GPU display. This program is associated with the
    // active GL4Engine object.  The returned program is valid only when the
    // program handle is positive.  The files are loaded, converted to
    // strings, and passed to CreateFromNamedSources.  The filenames are
    // passed as the 'xsName' parameters in case the shader compiler needs
    // this for #include path searches.
    virtual std::shared_ptr<VisualProgram> CreateFromNamedSources(
        std::string const& vsName, std::string const& vsSource,
        std::string const& psName, std::string const& psSource,
        std::string const& gsName, std::string const& gsSource);

    // Create a program for GPU computing.  This program is associated with
    // the active GL4Engine object.  The returned program is valid only when
    // the program handle is positive.  The file is loaded, converted to
    // a string, and passed to CreateFromNamedSource.  The filename is passed
    // as the 'csName' parameters in case the shader compiler needs this for
    // #include path searches.
    virtual std::shared_ptr<ComputeProgram> CreateFromNamedSource(
        std::string const& csName, std::string const& csSource);

    GLuint Compile(GLenum shaderType, std::string const& source);
    bool Link(GLuint programHandle);
};

}
