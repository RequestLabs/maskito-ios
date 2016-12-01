// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <Graphics/GteVisibleSet.h>
using namespace gte;


VisibleSet::~VisibleSet()
{
}

VisibleSet::VisibleSet(int numElements)
    :
    mNumVisible(0),
    mVisible(numElements > 0 ? numElements : INITIALLY_VISIBLE)
{
}

void VisibleSet::Insert(Visual* visible)
{
    if (mNumVisible < (int)mVisible.size())
    {
        mVisible[mNumVisible] = visible;
    }
    else
    {
        mVisible.push_back(visible);
    }
    ++mNumVisible;
}

bool VisibleSet::Remove(Visual* visible)
{
    for (int i = 0; i < mNumVisible; ++i)
    {
        if (visible == mVisible[i])
        {
            for (int j = i + 1; j < mNumVisible; ++j)
            {
                mVisible[i] = mVisible[j];
            }
            --mNumVisible;
            return true;
        }
    }
    return false;
}

bool VisibleSet::Contains(Visual* visible) const
{
    for (int i = 0; i < mNumVisible; ++i)
    {
        if (visible == mVisible[i])
        {
            return true;
        }
    }
    return false;
}

