// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <LowLevel/GteLogger.h>
#include <Graphics/GL4/GteGLSLReflection.h>
using namespace gte;


GLSLReflection::GLSLReflection(GLuint handle)
    :
    mHandle(handle)
{
    if (mHandle > 0)
    {
        ReflectProgramInputs();
        ReflectProgramOutputs();
        ReflectUniforms();
        ReflectDataBlocks(GL_UNIFORM_BLOCK, mUniformBlocks);
        ReflectDataBlocks(GL_SHADER_STORAGE_BLOCK, mShaderStorageBlocks);
        ReflectAtomicCounterBuffers();
        ReflectSubroutines(GL_VERTEX_SUBROUTINE, mVertexSubroutines);
        ReflectSubroutines(GL_GEOMETRY_SUBROUTINE, mGeometrySubroutines);
        ReflectSubroutines(GL_FRAGMENT_SUBROUTINE, mPixelSubroutines);
        ReflectSubroutines(GL_COMPUTE_SUBROUTINE, mComputeSubroutines);
        ReflectSubroutines(GL_TESS_CONTROL_SUBROUTINE,
            mTessControlSubroutines);
        ReflectSubroutines(GL_TESS_EVALUATION_SUBROUTINE,
            mTessEvaluationSubroutines);
        ReflectSubroutineUniforms(GL_VERTEX_SUBROUTINE_UNIFORM,
            mVertexSubroutineUniforms);
        ReflectSubroutineUniforms(GL_GEOMETRY_SUBROUTINE_UNIFORM,
            mGeometrySubroutineUniforms);
        ReflectSubroutineUniforms(GL_FRAGMENT_SUBROUTINE_UNIFORM,
            mPixelSubroutineUniforms);
        ReflectSubroutineUniforms(GL_COMPUTE_SUBROUTINE_UNIFORM,
            mComputeSubroutineUniforms);
        ReflectSubroutineUniforms(GL_TESS_CONTROL_SUBROUTINE_UNIFORM,
            mTessControlSubroutineUniforms);
        ReflectSubroutineUniforms(GL_TESS_EVALUATION_SUBROUTINE_UNIFORM,
            mTessEvaluationSubroutineUniforms);
        ReflectBufferVariables();
        ReflectTransformFeedbackVaryings();
        ReflectTransformFeedbackBuffers();
    }
    else
    {
        LogError("The program handle is invalid.");
    }
}

void GLSLReflection::ReflectProgramInputs()
{
    GLint numResources = 0;
    glGetProgramInterfaceiv(mHandle, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES,
        &numResources);
    if (numResources > 0)
    {
        mInputs.resize(numResources);

        GLenum properties[] =
        {
            GL_NAME_LENGTH,
            GL_TYPE,
            GL_LOCATION,
            GL_ARRAY_SIZE,
            GL_REFERENCED_BY_VERTEX_SHADER,
            GL_REFERENCED_BY_GEOMETRY_SHADER,
            GL_REFERENCED_BY_FRAGMENT_SHADER,
            GL_REFERENCED_BY_COMPUTE_SHADER,
            GL_REFERENCED_BY_TESS_CONTROL_SHADER,
            GL_REFERENCED_BY_TESS_EVALUATION_SHADER,
            GL_IS_PER_PATCH,
            // This property used to cause glGetProgramResourceiv to generate
            // a GL_INVALID_ENUM error with AMD Radeon HD 7900 series.  The
            // call is successful as of driver version 4.4.13283 Compatibility
            // Profile Context 14.501.1003.0.
            GL_LOCATION_COMPONENT
        };
        int const numProperties = sizeof(properties) / sizeof(int);
        GLint results[numProperties] = { 0 };
        for (int i = 0; i < numResources; ++i)
        {
            Input& info = mInputs[i];
            glGetProgramResourceiv(mHandle, GL_PROGRAM_INPUT, i,
                numProperties, properties, numProperties, nullptr, results);

            GLint numBytes = results[0] + 1;
            GLchar* name = new GLchar[numBytes];
            glGetProgramResourceName(mHandle, GL_PROGRAM_INPUT, i, numBytes,
                nullptr, name);
            info.name = std::string(name);
            delete[] name;

            GLint* current = &results[1];
            info.type = *current++;
            info.location = *current++;
            info.arraySize = *current++;
            info.referencedBy[ST_VERTEX] = *current++;
            info.referencedBy[ST_GEOMETRY] = *current++;
            info.referencedBy[ST_PIXEL] = *current++;
            info.referencedBy[ST_COMPUTE] = *current++;
            info.referencedBy[ST_TESSCONTROL] = *current++;
            info.referencedBy[ST_TESSEVALUATION] = *current++;
            info.isPerPatch = *current++;
            info.locationComponent = *current++;
        }
    }
}

void GLSLReflection::ReflectProgramOutputs()
{
    GLint numResources = 0;
    glGetProgramInterfaceiv(mHandle, GL_PROGRAM_OUTPUT, GL_ACTIVE_RESOURCES,
        &numResources);
    if (numResources > 0)
    {
        mOutputs.resize(numResources);

        GLenum properties[] =
        {
            GL_NAME_LENGTH,
            GL_TYPE,
            GL_LOCATION,
            GL_ARRAY_SIZE,
            GL_REFERENCED_BY_VERTEX_SHADER,
            GL_REFERENCED_BY_GEOMETRY_SHADER,
            GL_REFERENCED_BY_FRAGMENT_SHADER,
            GL_REFERENCED_BY_COMPUTE_SHADER,
            GL_REFERENCED_BY_TESS_CONTROL_SHADER,
            GL_REFERENCED_BY_TESS_EVALUATION_SHADER,
            GL_IS_PER_PATCH,
            // This property used to cause glGetProgramResourceiv to generate
            // a GL_INVALID_ENUM error with AMD Radeon HD 7900 series.  The
            // call is successful as of driver version 4.4.13283 Compatibility
            // Profile Context 14.501.1003.0.
            GL_LOCATION_COMPONENT,
            GL_LOCATION_INDEX
        };
        int const numProperties = sizeof(properties) / sizeof(int);
        GLint results[numProperties] = { 0 };
        for (int i = 0; i < numResources; ++i)
        {
            Output& info = mOutputs[i];
            glGetProgramResourceiv(mHandle, GL_PROGRAM_OUTPUT, i,
                numProperties, properties, numProperties, nullptr, results);

            GLint numBytes = results[0] + 1;
            GLchar* name = new GLchar[numBytes];
            glGetProgramResourceName(mHandle, GL_PROGRAM_OUTPUT, i, numBytes,
                nullptr, name);
            info.name = std::string(name);
            delete[] name;

            GLint* current = &results[1];
            info.type = *current++;
            info.location = *current++;
            info.arraySize = *current++;
            info.referencedBy[ST_VERTEX] = *current++;
            info.referencedBy[ST_GEOMETRY] = *current++;
            info.referencedBy[ST_PIXEL] = *current++;
            info.referencedBy[ST_COMPUTE] = *current++;
            info.referencedBy[ST_TESSCONTROL] = *current++;
            info.referencedBy[ST_TESSEVALUATION] = *current++;
            info.isPerPatch = *current++;
            info.locationComponent = *current++;
            info.locationIndex = *current++;
        }
    }
}

void GLSLReflection::ReflectUniforms()
{
    GLint numResources = 0;
    glGetProgramInterfaceiv(mHandle, GL_UNIFORM, GL_ACTIVE_RESOURCES,
        &numResources);

    if (numResources > 0)
    {
        mUniforms.resize(numResources);

        GLenum properties[] =
        {
            GL_NAME_LENGTH,
            GL_TYPE,
            GL_LOCATION,
            GL_ARRAY_SIZE,
            GL_OFFSET,
            GL_BLOCK_INDEX,
            GL_ARRAY_STRIDE,
            GL_MATRIX_STRIDE,
            GL_IS_ROW_MAJOR,
            GL_ATOMIC_COUNTER_BUFFER_INDEX,
            GL_REFERENCED_BY_VERTEX_SHADER,
            GL_REFERENCED_BY_GEOMETRY_SHADER,
            GL_REFERENCED_BY_FRAGMENT_SHADER,
            GL_REFERENCED_BY_COMPUTE_SHADER,
            GL_REFERENCED_BY_TESS_CONTROL_SHADER,
            GL_REFERENCED_BY_TESS_EVALUATION_SHADER
        };
        int const numProperties = sizeof(properties) / sizeof(int);
        GLint results[numProperties] = { 0 };
        for (int i = 0; i < numResources; ++i)
        {
            Uniform& info = mUniforms[i];
            glGetProgramResourceiv(mHandle, GL_UNIFORM, i,
                numProperties, properties, numProperties, nullptr, results);

            GLint numBytes = results[0] + 1;
            GLchar* name = new GLchar[numBytes];
            glGetProgramResourceName(mHandle, GL_UNIFORM, i, numBytes,
                nullptr, name);
            info.name = std::string(name);
            delete[] name;

            GLint* current = &results[1];
            info.type = *current++;
            info.location = *current++;
            info.arraySize = *current++;
            info.offset = *current++;
            info.blockIndex = *current++;
            info.arrayStride = *current++;
            info.matrixStride = *current++;
            info.isRowMajor = *current++;
            info.atomicCounterBufferIndex = *current++;
            info.referencedBy[ST_VERTEX] = *current++;
            info.referencedBy[ST_GEOMETRY] = *current++;
            info.referencedBy[ST_PIXEL] = *current++;
            info.referencedBy[ST_COMPUTE] = *current++;
            info.referencedBy[ST_TESSCONTROL] = *current++;
            info.referencedBy[ST_TESSEVALUATION] = *current++;

            // For an array member, the name is of the form "someName[0]".
            // The GTEngine Shader class currently wants "someName" because
            // that is how HLSL delivered the name.  Let's conform to that
            // for now.
            if (info.arraySize > 1)
            {
                auto index = info.name.find("[0]");
                if (index != std::string::npos)
                {
                    info.name = info.name.substr(0, index);
                }
                else
                {
                    // TODO: For now, trap any occurrence of an array member
                    // whose name is not of the form "someName[0]".
                    LogError("Unexpected condition.");
                }
            }
        }
    }
}

void GLSLReflection::ReflectDataBlocks(GLenum programInterface,
    std::vector<DataBlock>& blocks)
{
    GLint numResources = 0;
    glGetProgramInterfaceiv(mHandle, programInterface, GL_ACTIVE_RESOURCES,
        &numResources);

    if (numResources > 0)
    {
        blocks.resize(numResources);

        GLenum properties[] =
        {
            GL_NAME_LENGTH,
            GL_BUFFER_BINDING,
            GL_BUFFER_DATA_SIZE,
            GL_REFERENCED_BY_VERTEX_SHADER,
            GL_REFERENCED_BY_GEOMETRY_SHADER,
            GL_REFERENCED_BY_FRAGMENT_SHADER,
            GL_REFERENCED_BY_COMPUTE_SHADER,
            GL_REFERENCED_BY_TESS_CONTROL_SHADER,
            GL_REFERENCED_BY_TESS_EVALUATION_SHADER,
            GL_NUM_ACTIVE_VARIABLES
        };
        int const numProperties = sizeof(properties) / sizeof(int);
        GLint results[numProperties] = { 0 };
        for (int i = 0; i < numResources; ++i)
        {
            DataBlock& info = blocks[i];
            glGetProgramResourceiv(mHandle, programInterface, i,
                numProperties, properties, numProperties, nullptr, results);

            GLint numBytes = results[0] + 1;
            GLchar* name = new GLchar[numBytes];
            glGetProgramResourceName(mHandle, programInterface, i, numBytes,
                nullptr, name);
            info.name = std::string(name);
            delete[] name;

            GLint* current = &results[1];
            info.bufferBinding = *current++;
            info.bufferDataSize = *current++;
            info.referencedBy[ST_VERTEX] = *current++;
            info.referencedBy[ST_GEOMETRY] = *current++;
            info.referencedBy[ST_PIXEL] = *current++;
            info.referencedBy[ST_COMPUTE] = *current++;
            info.referencedBy[ST_TESSCONTROL] = *current++;
            info.referencedBy[ST_TESSEVALUATION] = *current++;

            GLint numActiveVariables = *current++;
            if (numActiveVariables > 0)
            {
                info.activeVariables.resize(numActiveVariables);
                std::fill(info.activeVariables.begin(),
                    info.activeVariables.end(), 0);
                GLenum varProperty = GL_ACTIVE_VARIABLES;
                glGetProgramResourceiv(mHandle, programInterface, i, 1,
                    &varProperty, numActiveVariables, nullptr,
                    &info.activeVariables[0]);
            }
        }
    }
}

void GLSLReflection::ReflectAtomicCounterBuffers()
{
    GLint numResources = 0;
    glGetProgramInterfaceiv(mHandle, GL_ATOMIC_COUNTER_BUFFER,
        GL_ACTIVE_RESOURCES, &numResources);

    if (numResources > 0)
    {
        mAtomicCounterBuffers.resize(numResources);

        GLenum properties[] =
        {
            GL_BUFFER_BINDING,
            GL_BUFFER_DATA_SIZE,
            GL_REFERENCED_BY_VERTEX_SHADER,
            GL_REFERENCED_BY_GEOMETRY_SHADER,
            GL_REFERENCED_BY_FRAGMENT_SHADER,
            GL_REFERENCED_BY_COMPUTE_SHADER,
            GL_REFERENCED_BY_TESS_CONTROL_SHADER,
            GL_REFERENCED_BY_TESS_EVALUATION_SHADER,
            GL_NUM_ACTIVE_VARIABLES
        };
        int const numProperties = sizeof(properties) / sizeof(int);
        GLint results[numProperties] = { 0 };
        for (int i = 0; i < numResources; ++i)
        {
            AtomicCounterBuffer& info = mAtomicCounterBuffers[i];
            glGetProgramResourceiv(mHandle, GL_ATOMIC_COUNTER_BUFFER, i,
                numProperties, properties, numProperties, nullptr, results);

            GLint* current = &results[0];
            info.bufferBinding = *current++;
            info.bufferDataSize = *current++;
            info.referencedBy[ST_VERTEX] = *current++;
            info.referencedBy[ST_GEOMETRY] = *current++;
            info.referencedBy[ST_PIXEL] = *current++;
            info.referencedBy[ST_COMPUTE] = *current++;
            info.referencedBy[ST_TESSCONTROL] = *current++;
            info.referencedBy[ST_TESSEVALUATION] = *current++;

            GLint numActiveVariables = *current++;
            if (numActiveVariables > 0)
            {
                info.activeVariables.resize(numActiveVariables);
                std::fill(info.activeVariables.begin(),
                    info.activeVariables.end(), 0);
                GLenum varProperty = GL_ACTIVE_VARIABLES;
                glGetProgramResourceiv(mHandle, GL_ATOMIC_COUNTER_BUFFER, i,
                    1, &varProperty, numActiveVariables, nullptr,
                    &info.activeVariables[0]);
            }
        }
    }
}

void GLSLReflection::ReflectSubroutines(GLenum programInterface,
    std::vector<std::string>& subroutines)
{
    GLint numResources = 0;
    glGetProgramInterfaceiv(mHandle, programInterface, GL_ACTIVE_RESOURCES,
        &numResources);
    if (numResources > 0)
    {
        subroutines.resize(numResources);

        GLenum nameLengthProperty = GL_NAME_LENGTH;
        for (int i = 0; i < numResources; ++i)
        {
            GLint result = 0;
            glGetProgramResourceiv(mHandle, programInterface, i, 1,
                &nameLengthProperty, 1, nullptr, &result);

            GLint numBytes = result + 1;
            GLchar* name = new GLchar[numBytes];
            glGetProgramResourceName(mHandle, programInterface, i, numBytes,
                nullptr, name);
            subroutines[i] = std::string(name);
            delete[] name;
        }
    }
}

void GLSLReflection::ReflectSubroutineUniforms(GLenum programInterface,
    std::vector<SubroutineUniform>& subUniforms)
{
    GLint numResources = 0;
    glGetProgramInterfaceiv(mHandle, programInterface, GL_ACTIVE_RESOURCES,
        &numResources);

    if (numResources > 0)
    {
        subUniforms.resize(numResources);

        GLenum properties[] =
        {
            GL_NAME_LENGTH,
            GL_LOCATION,
            GL_ARRAY_SIZE,
            GL_NUM_ACTIVE_VARIABLES
        };
        int const numProperties = sizeof(properties) / sizeof(int);
        GLint results[numProperties] = { 0 };
        for (int i = 0; i < numResources; ++i)
        {
            SubroutineUniform& info = subUniforms[i];
            glGetProgramResourceiv(mHandle, programInterface, i,
                numProperties, properties, numProperties, nullptr, results);

            GLint numBytes = results[0] + 1;
            GLchar* name = new GLchar[numBytes];
            glGetProgramResourceName(mHandle, programInterface, i,
                numBytes, nullptr, name);
            info.name = std::string(name);
            delete[] name;

            GLint* current = &results[1];
            info.location = *current++;
            info.arraySize = *current++;

            GLint numCompatibleSubroutines = *current++;
            if (numCompatibleSubroutines > 0)
            {
                info.compatibleSubroutines.resize(numCompatibleSubroutines);
                std::fill(info.compatibleSubroutines.begin(),
                    info.compatibleSubroutines.end(), 0);
                GLenum subProperty = GL_COMPATIBLE_SUBROUTINES;
                glGetProgramResourceiv(mHandle, programInterface, i, 1,
                    &subProperty, numCompatibleSubroutines, nullptr,
                    &info.compatibleSubroutines[0]);
            }
        }
    }
}

void GLSLReflection::ReflectBufferVariables()
{
    GLint numResources = 0;
    glGetProgramInterfaceiv(mHandle, GL_BUFFER_VARIABLE, GL_ACTIVE_RESOURCES,
        &numResources);
    if (numResources > 0)
    {
        mBufferVariables.resize(numResources);

        GLenum properties[] =
        {
            GL_NAME_LENGTH,
            GL_TYPE,
            GL_ARRAY_SIZE,
            GL_OFFSET,
            GL_BLOCK_INDEX,
            GL_ARRAY_STRIDE,
            GL_MATRIX_STRIDE,
            GL_IS_ROW_MAJOR,
            GL_TOP_LEVEL_ARRAY_SIZE,
            GL_TOP_LEVEL_ARRAY_STRIDE,
            GL_REFERENCED_BY_VERTEX_SHADER,
            GL_REFERENCED_BY_GEOMETRY_SHADER,
            GL_REFERENCED_BY_FRAGMENT_SHADER,
            GL_REFERENCED_BY_COMPUTE_SHADER,
            GL_REFERENCED_BY_TESS_CONTROL_SHADER,
            GL_REFERENCED_BY_TESS_EVALUATION_SHADER
        };
        int const numProperties = sizeof(properties) / sizeof(int);
        GLint results[numProperties] = { 0 };
        for (int i = 0; i < numResources; ++i)
        {
            BufferVariable& info = mBufferVariables[i];
            glGetProgramResourceiv(mHandle, GL_BUFFER_VARIABLE, i,
                numProperties, properties, numProperties, nullptr, results);

            GLint numBytes = results[0] + 1;
            GLchar* name = new GLchar[numBytes];
            glGetProgramResourceName(mHandle, GL_BUFFER_VARIABLE, i, numBytes,
                nullptr, name);
            info.name = std::string(name);
            delete[] name;

            GLint* current = &results[1];
            info.type = *current++;
            info.arraySize = *current++;
            info.offset = *current++;
            info.blockIndex = *current++;
            info.arrayStride = *current++;
            info.matrixStride = *current++;
            info.isRowMajor = *current++;
            info.topLevelArraySize = *current++;
            info.topLevelArrayStride = *current++;
            info.referencedBy[ST_VERTEX] = *current++;
            info.referencedBy[ST_GEOMETRY] = *current++;
            info.referencedBy[ST_PIXEL] = *current++;
            info.referencedBy[ST_COMPUTE] = *current++;
            info.referencedBy[ST_TESSCONTROL] = *current++;
            info.referencedBy[ST_TESSEVALUATION] = *current++;
        }
    }
}

void GLSLReflection::ReflectTransformFeedbackVaryings()
{
    GLint numResources = 0;
    glGetProgramInterfaceiv(mHandle, GL_TRANSFORM_FEEDBACK_VARYING,
        GL_ACTIVE_RESOURCES, &numResources);
    if (numResources > 0)
    {
        mInputs.resize(numResources);

        GLenum properties[] =
        {
            GL_NAME_LENGTH,
            GL_TYPE,
            GL_ARRAY_SIZE,
            GL_OFFSET,
            GL_TRANSFORM_FEEDBACK_BUFFER_INDEX
        };
        int const numProperties = sizeof(properties) / sizeof(int);
        GLint results[numProperties] = { 0 };
        for (int i = 0; i < numResources; ++i)
        {
            TransformFeedbackVarying& info = mTransformFeedbackVaryings[i];
            glGetProgramResourceiv(mHandle, GL_TRANSFORM_FEEDBACK_VARYING, i,
                numProperties, properties, numProperties, nullptr, results);

            GLint numBytes = results[0] + 1;
            GLchar* name = new GLchar[numBytes];
            glGetProgramResourceName(mHandle, GL_TRANSFORM_FEEDBACK_VARYING,
                i, numBytes, nullptr, name);
            info.name = std::string(name);
            delete[] name;

            GLint* current = &results[1];
            info.type = *current++;
            info.arraySize = *current++;
            info.offset = *current++;
            info.transformFeedbackBufferIndex = *current++;
        }
    }
}

void GLSLReflection::ReflectTransformFeedbackBuffers()
{
    // This property causes glGetProgramInterfaceiv to generate a
    // GL_INVALID_ENUM error with AMD Radeon HD 7900 series, driver version
    // 4.4.13283 Compatibility Profile Context 14.501.1003.0 (and previous
    // versions).  NVIDIA drivers appear to have no problems with this.
#if 0
    GLint numResources = 0;
    glGetProgramInterfaceiv(mHandle, GL_TRANSFORM_FEEDBACK_BUFFER,
        GL_ACTIVE_RESOURCES, &numResources);
    if (numResources > 0)
    {
        mTransformFeedbackBuffers.resize(numResources);

        GLenum properties[] =
        {
            GL_BUFFER_BINDING,
            GL_TRANSFORM_FEEDBACK_BUFFER_STRIDE,
            GL_NUM_ACTIVE_VARIABLES
        };
        int const numProperties = sizeof(properties) / sizeof(int);
        GLint results[numProperties] = { 0 };
        for (int i = 0; i < numResources; ++i)
        {
            TransformFeedbackBuffer& info = mTransformFeedbackBuffers[i];
            glGetProgramResourceiv(mHandle, GL_TRANSFORM_FEEDBACK_BUFFER, i,
                numProperties, properties, numProperties, nullptr, results);

            GLint* current = &results[0];
            info.bufferBinding = *current++;
            info.transformFeedbackBufferStride = *current++;

            GLint numActiveVariables = *current++;
            if (numActiveVariables > 0)
            {
                info.activeVariables.resize(numActiveVariables);
                std::fill(info.activeVariables.begin(),
                    info.activeVariables.end(), 0);
                GLenum varProperty = GL_ACTIVE_VARIABLES;
                glGetProgramResourceiv(mHandle, GL_TRANSFORM_FEEDBACK_BUFFER,
                    i, 1, &varProperty, numActiveVariables, nullptr,
                    &info.activeVariables[0]);
            }
        }
    }
#endif
}

