// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#pragma once

#include <GTEngineDEF.h>
#include <vector>

namespace gte
{

// The VisibleSet class maintains an array of potentially visible objects in
// a scene.  The typical usage is to clear the array, insert potentially
// visible objects during a scene graph update, and then get the array for
// iteration by the renderer for drawing.  In some circumstances, you might
// want to remove an object from the array, but the remaining objects are
// still potentially visible.  Rather than applying a full scene graph update,
// a Remove function is provided.  It uses a linear search of the array,
// which sounds expensive; however, this is potentially less expensive than
// the full rebuild of the potentially visible set (via the update).  The
// array is grow-only, which avoids the dynamic memory costs that can occur
// frequently otherwise in a real-time application.

class Visual;

class GTE_IMPEXP VisibleSet
{
public:
    // Construction and destruction.
    enum { INITIALLY_VISIBLE = 128 };
    ~VisibleSet();
    VisibleSet(int numElements = INITIALLY_VISIBLE);

    // Access to the elements of the visible set.  Only the first
    // GetNumVisible() elements are valid; thus, you should iterate over the
    // array only through the specified number.
    inline int GetNumVisible() const;
    inline Visual* Get(int i) const;

    // Support for modifying the visible set.  The Clear function sets the
    // number of visible objects to zero but does not resize the array.  The
    // Resize function does (potentially) change the array allocation.
    inline void Clear();
    inline void Resize(int numElements);
    void Insert(Visual* visible);
    bool Remove(Visual* visible);
    bool Contains(Visual* visible) const;

private:
    int mNumVisible;
    std::vector<Visual*> mVisible;
};


inline int VisibleSet::GetNumVisible() const
{
    return mNumVisible;
}

inline Visual* VisibleSet::Get(int i) const
{
    if (0 <= i && i < mNumVisible)
    {
        return mVisible[i];
    }
    else
    {
        return 0;
    }
}

inline void VisibleSet::Clear()
{
    mNumVisible = 0;
}

inline void VisibleSet::Resize(int numElements)
{
    mVisible.resize(numElements > 0 ? numElements : INITIALLY_VISIBLE);
}


}
