// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <Graphics/DX11/GteDX11Include.h>
#include <fstream>

namespace gte
{

class GTE_IMPEXP HLSLResource
{
public:
    // Abstract base class, destructor.
    virtual ~HLSLResource();
protected:
    // Construction.
    HLSLResource(D3D11_SHADER_INPUT_BIND_DESC const& desc,
        unsigned int numBytes);
    HLSLResource(D3D11_SHADER_INPUT_BIND_DESC const& desc, unsigned int index,
        unsigned int numBytes);

public:
    struct Description
    {
        std::string name;
        D3D_SHADER_INPUT_TYPE type;
        unsigned int bindPoint;
        unsigned int bindCount;
        unsigned int flags;
        D3D_RESOURCE_RETURN_TYPE returnType;
        D3D_SRV_DIMENSION dimension;
        unsigned int numSamples;
    };

    // Member access.
    std::string const& GetName() const;
    D3D_SHADER_INPUT_TYPE GetType() const;
    unsigned int GetBindPoint() const;
    unsigned int GetBindCount() const;
    unsigned int GetFlags() const;
    D3D_RESOURCE_RETURN_TYPE GetReturnType() const;
    D3D_SRV_DIMENSION GetDimension() const;
    unsigned int GetNumSamples() const;
    unsigned int GetNumBytes() const;

    // Print to a text file for human readability.
    virtual void Print(std::ofstream& output) const;

private:
    Description mDesc;
    unsigned int mNumBytes;

    // Support for Print.
    static std::string const msSIType[];
    static std::string const msReturnType[];
    static std::string const msSRVDimension[];
};

}
