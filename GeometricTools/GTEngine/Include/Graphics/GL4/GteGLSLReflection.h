// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GL4/GteOpenGL.h>
#include <string>
#include <vector>

// Query a program object for all information relevant to manipulating the
// program at run time.

namespace gte
{

class GTE_IMPEXP GLSLReflection
{
public:
    // Construction.  The input is the handle to a program that was
    // successfully created for the active context.
    GLSLReflection(GLuint handle);

    enum  // Named indices for the 'referencedBy' arrays.
    {
        ST_VERTEX,
        ST_GEOMETRY,
        ST_PIXEL,
        ST_COMPUTE,
        ST_TESSCONTROL,
        ST_TESSEVALUATION
    };

    struct Input
    {
        std::string name;
        GLint type;
        GLint location;
        GLint arraySize;
        GLint referencedBy[6];
        GLint isPerPatch;
        GLint locationComponent;
    };

    struct Output
    {
        std::string name;
        GLint type;
        GLint location;
        GLint arraySize;
        GLint referencedBy[6];
        GLint isPerPatch;
        GLint locationComponent;
        GLint locationIndex;
    };

    struct Uniform
    {
        std::string name;
        GLint type;
        GLint location;
        GLint arraySize;
        GLint offset;
        GLint blockIndex;
        GLint arrayStride;
        GLint matrixStride;
        GLint isRowMajor;
        GLint atomicCounterBufferIndex;
        GLint referencedBy[6];
    };

    struct DataBlock
    {
        std::string name;
        GLint bufferBinding;
        GLint bufferDataSize;
        GLint referencedBy[6];
        std::vector<GLint> activeVariables;
    };

    struct AtomicCounterBuffer
    {
        GLint bufferBinding;
        GLint bufferDataSize;
        GLint referencedBy[6];
        std::vector<GLint> activeVariables;
    };

    struct SubroutineUniform
    {
        std::string name;
        GLint location;
        GLint arraySize;
        std::vector<GLint> compatibleSubroutines;
    };

    struct BufferVariable
    {
        std::string name;
        GLint type;
        GLint arraySize;
        GLint offset;
        GLint blockIndex;
        GLint arrayStride;
        GLint matrixStride;
        GLint isRowMajor;
        GLint topLevelArraySize;
        GLint topLevelArrayStride;
        GLint referencedBy[6];
    };

    struct TransformFeedbackVarying
    {
        std::string name;
        GLint type;
        GLint arraySize;
        GLint offset;
        GLint transformFeedbackBufferIndex;
    };

    struct TransformFeedbackBuffer
    {
        GLint bufferBinding;
        GLint transformFeedbackBufferStride;
        std::vector<GLint> activeVariables;
    };

    // Member access.
    inline GLuint GetProgramHandle() const;
    inline std::vector<Input> const& GetInputs() const;
    inline std::vector<Output> const& GetOutputs() const;
    inline std::vector<Uniform> const& GetUniforms() const;
    inline std::vector<DataBlock> const& GetUniformBlocks() const;

private:
    void ReflectProgramInputs();
    void ReflectProgramOutputs();
    void ReflectUniforms();
    void ReflectDataBlocks(GLenum programInterface,
        std::vector<DataBlock>& blocks);
    void ReflectAtomicCounterBuffers();
    void ReflectSubroutines(GLenum programInterface,
        std::vector<std::string>& subroutines);
    void ReflectSubroutineUniforms(GLenum programInterface,
        std::vector<SubroutineUniform>& subUniforms);
    void ReflectBufferVariables();
    void ReflectTransformFeedbackVaryings();
    void ReflectTransformFeedbackBuffers();

    GLuint mHandle;
    std::vector<Input> mInputs;
    std::vector<Output> mOutputs;
    std::vector<Uniform> mUniforms;
    std::vector<DataBlock> mUniformBlocks;
    std::vector<DataBlock> mShaderStorageBlocks;
    std::vector<AtomicCounterBuffer> mAtomicCounterBuffers;
    std::vector<std::string> mVertexSubroutines;
    std::vector<std::string> mGeometrySubroutines;
    std::vector<std::string> mPixelSubroutines;
    std::vector<std::string> mComputeSubroutines;
    std::vector<std::string> mTessControlSubroutines;
    std::vector<std::string> mTessEvaluationSubroutines;
    std::vector<SubroutineUniform> mVertexSubroutineUniforms;
    std::vector<SubroutineUniform> mGeometrySubroutineUniforms;
    std::vector<SubroutineUniform> mPixelSubroutineUniforms;
    std::vector<SubroutineUniform> mComputeSubroutineUniforms;
    std::vector<SubroutineUniform> mTessControlSubroutineUniforms;
    std::vector<SubroutineUniform> mTessEvaluationSubroutineUniforms;
    std::vector<BufferVariable> mBufferVariables;
    std::vector<TransformFeedbackVarying> mTransformFeedbackVaryings;
    std::vector<TransformFeedbackBuffer> mTransformFeedbackBuffers;
};


inline GLuint GLSLReflection::GetProgramHandle() const
{
    return mHandle;
}

inline std::vector<GLSLReflection::Input> const&
GLSLReflection::GetInputs() const
{
    return mInputs;
}

inline std::vector<GLSLReflection::Output> const&
GLSLReflection::GetOutputs() const
{
    return mOutputs;
}

inline std::vector<GLSLReflection::Uniform> const&
GLSLReflection::GetUniforms() const
{
    return mUniforms;
}

inline std::vector<GLSLReflection::DataBlock> const&
GLSLReflection::GetUniformBlocks() const
{
    return mUniformBlocks;
}



}
