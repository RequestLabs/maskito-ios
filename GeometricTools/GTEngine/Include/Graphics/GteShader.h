// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/GteConstantBuffer.h>
#include <Graphics/GteRawBuffer.h>
#include <Graphics/GteSamplerState.h>
#include <Graphics/GteStructuredBuffer.h>
#include <Graphics/GteTexture1.h>
#include <Graphics/GteTexture2.h>
#include <Graphics/GteTexture3.h>
#include <Graphics/GteTextureBuffer.h>
#include <Graphics/GteTextureCube.h>
#include <Graphics/GteTexture1Array.h>
#include <Graphics/GteTexture2Array.h>
#include <Graphics/GteTextureCubeArray.h>
#include <Graphics/DX11/GteHLSLShader.h>
#if defined(GTE_DEV_OPENGL)
#include <Graphics/GL4/GteGLSLReflection.h>
#endif
#include <memory>

namespace gte
{

class GTE_IMPEXP Shader : public GraphicsObject
{
public:
    // Construction for shaders corresponding to HLSL.
    Shader(HLSLShader const& program);

#if defined(GTE_DEV_OPENGL)
    // Constructon for shaders corresponding to GLSL.  The 'type' parameter
    // in the constructor is one of the enumerates in GLSLReflection:
    // ST_VERTEX, ST_GEOMETRY, ST_PIXEL, ST_COMPUTE.
    Shader(GLSLReflection const& reflector, int type);
#endif

    // To avoid frequent string comparisons during run time, obtain a handle
    // for an object and use it instead for setting and getting the values.
    // If the named object exists, the returned handle is nonnegative;
    // otherwise, it is -1.
    int Get(std::string const& name) const;

    // Set or get the buffers.  If the set is successful, the return value is
    // nonnegative and is the index into the appropriate array.  This handle
    // may be passed to the Set(handle,*) and Get(handle,*) functions.  The
    // mechanism allows you to set directly by index and avoid the name
    // comparisons that occur with the Set(name,*) and Get(name,*) functions.
    template <typename T>
    int Set(std::string const& name, std::shared_ptr<T> const& object);

    template <typename T>
    std::shared_ptr<T> const Get(std::string const& name) const;

    template <typename T>
    void Set(int handle, std::shared_ptr<T> const& object);

    template <typename T>
    std::shared_ptr<T> const Get(int handle) const;

    // These specializations copy the member layouts of the shader program to the
    // buffer objects.
    template<>
    int Set(std::string const& name, std::shared_ptr<ConstantBuffer> const& object);

    template<>
    int Set(std::string const& name, std::shared_ptr<TextureBuffer> const& object);

    template<>
    void Set(int handle, std::shared_ptr<ConstantBuffer> const& object);

    template<>
    void Set(int handle, std::shared_ptr<TextureBuffer> const& object);

    inline unsigned int GetNumXThreads() const;
    inline unsigned int GetNumYThreads() const;
    inline unsigned int GetNumZThreads() const;

protected:
    // This structure provides the data necessary for the engine to attach
    // the associated resources to the shader, including name lookups and
    // resource validation.  Not all members are used for each graphics
    // object type:
    //   CB - constant buffer, lookup 0
    //   TB - texture buffer, lookup 1
    //   SB - structured buffer (and derived classes), lookup 2
    //   TODO:  typed buffer
    //   RB - raw buffer, lookup 3
    //   TX - texture (and derived classes), lookup 4
    //   TA - texture array (and derived classes, lookup 5
    //   SS - sampler state, lookup 6
    struct Data
    {
        Data(GraphicsObjectType inType, std::string const& inName,
            int inBindPoint, int inNumBytes, unsigned int inExtra,
            bool inIsGpuWritable);

        std::shared_ptr<GraphicsObject> object; // CB, TB, SB, RB, TX, TA, SS
        GraphicsObjectType type;                // CB, TB, SB, RB, TX, TA, SS
        std::string name;                       // CB, TB, SB, RB, TX, TA, SS
        int bindPoint;                          // CB, TB, SB, RB, TX, TA, SS
        int numBytes;                           // CB, TB, SB, RB
        unsigned int extra;                     // TX, TA (dims), SB (ctrtype)
        bool isGpuWritable;                     // SB, RB, TX, TA
    };

    bool IsValid(Data const& goal, ConstantBuffer* resource) const;
    bool IsValid(Data const& goal, TextureBuffer* resource) const;
    bool IsValid(Data const& goal, StructuredBuffer* resource) const;
    bool IsValid(Data const& goal, RawBuffer* resource) const;
    bool IsValid(Data const& goal, TextureSingle* resource) const;
    bool IsValid(Data const& goal, TextureArray* resource) const;
    bool IsValid(Data const& goal, SamplerState* state) const;

    enum { NUM_LOOKUP_INDICES = 7 };  // CB, TB, SB, RB, TX, TA, SS
    std::vector<Data> mData[NUM_LOOKUP_INDICES];
    std::vector<unsigned char> mCompiledCode;
    unsigned int mNumXThreads;
    unsigned int mNumYThreads;
    unsigned int mNumZThreads;

private:
    std::vector<std::vector<MemberLayout>> mCBufferLayouts;
    std::vector<std::vector<MemberLayout>> mTBufferLayouts;

public:
    // For use by the graphics engine.
    inline std::vector<unsigned char> const& GetCompiledCode() const;
    inline std::vector<Data> const& GetData(int lookup) const;
};


template <typename T>
int Shader::Set(std::string const& name, std::shared_ptr<T> const& object)
{
    int handle = 0;
    for (auto& data : mData[T::shaderDataLookup])
    {
        if (name == data.name)
        {
            if (IsValid(data, object.get()))
            {
                data.object = object;
                return handle;
            }
            return -1;
        }
        ++handle;
    }

    LogError("Cannot find object " + name + ".");
    return -1;
}

template <typename T>
std::shared_ptr<T> const Shader::Get(std::string const& name) const
{
    for (auto const& data : mData[T::shaderDataLookup])
    {
        if (name == data.name)
        {
            return std::static_pointer_cast<T>(data.object);
        }
    }

    LogError("Cannot find object " + name + ".");
    return nullptr;
}

template <typename T>
void Shader::Set(int handle, std::shared_ptr<T> const& object)
{
    std::vector<Data>& data = mData[T::shaderDataLookup];
    if (0 <= handle && handle < static_cast<int>(data.size()))
    {
        auto& d = data[handle];
        if (IsValid(d, object.get()))
        {
            d.object = object;
        }
        return;
    }

    LogError("Invalid handle for object.");
}

template <typename T>
std::shared_ptr<T> const Shader::Get(int handle) const
{
    std::vector<Data> const& data = mData[T::shaderDataLookup];
    if (0 <= handle && handle < static_cast<int>(data.size()))
    {
        return data[handle].object;
    }

    LogError("Cannot find object.");
    return nullptr;
}

template <>
int Shader::Set(std::string const& name,
    std::shared_ptr<ConstantBuffer> const& object)
{
    int handle = 0;
    for (auto& data : mData[ConstantBuffer::shaderDataLookup])
    {
        if (name == data.name)
        {
            if (IsValid(data, object.get()))
            {
                data.object = object;
                object->mLayout = mCBufferLayouts[handle];
                return handle;
            }
            return -1;
        }
        ++handle;
    }

    LogError("Cannot find object " + name + ".");
    return -1;
}

template <>
int Shader::Set(std::string const& name,
    std::shared_ptr<TextureBuffer> const& object)
{
    int handle = 0;
    for (auto& data : mData[TextureBuffer::shaderDataLookup])
    {
        if (name == data.name)
        {
            if (IsValid(data, object.get()))
            {
                data.object = object;
                object->mLayout = mTBufferLayouts[handle];
                return handle;
            }
            return -1;
        }
        ++handle;
    }

    LogError("Cannot find object " + name + ".");
    return -1;
}

template<>
void Shader::Set(int handle, std::shared_ptr<ConstantBuffer> const& object)
{
    std::vector<Data>& data = mData[ConstantBuffer::shaderDataLookup];
    if (0 <= handle && handle < static_cast<int>(data.size()))
    {
        auto& d = data[handle];
        if (IsValid(d, object.get()))
        {
            d.object = object;
            object->mLayout = mCBufferLayouts[handle];
        }
        return;
    }

    LogError("Invalid handle for object.");
}

template<>
void Shader::Set(int handle, std::shared_ptr<TextureBuffer> const& object)
{
    std::vector<Data>& data = mData[TextureBuffer::shaderDataLookup];
    if (0 <= handle && handle < static_cast<int>(data.size()))
    {
        auto& d = data[handle];
        if (IsValid(d, object.get()))
        {
            d.object = object;
            object->mLayout = mTBufferLayouts[handle];
        }
        return;
    }

    LogError("Invalid handle for object.");
}

inline unsigned int Shader::GetNumXThreads() const
{
    return mNumXThreads;
}

inline unsigned int Shader::GetNumYThreads() const
{
    return mNumYThreads;
}

inline unsigned int Shader::GetNumZThreads() const
{
    return mNumZThreads;
}

inline std::vector<unsigned char> const& Shader::GetCompiledCode() const
{
    return mCompiledCode;
}

inline std::vector<Shader::Data> const& Shader::GetData(int lookup) const
{
    return mData[lookup];
}


}
