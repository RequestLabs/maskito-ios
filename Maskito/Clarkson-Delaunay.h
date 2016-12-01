/* Clarkson-Delaunay.h */
/*
 * Ken Clarkson wrote this.  Copyright (c) 1995 by AT&T..
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 */
#ifndef Clarkson_Delaunay_h
#define Clarkson_Delaunay_h


#ifdef __cplusplus
extern "C" {
#endif
    unsigned int * BuildTriangleIndexList (void *pointList, float factor, int numberOfInputPoints, int numDimensions, int clockwise, int *numTriangleVertices );
#ifdef __cplusplus
}
#endif

#endif /* Clarkson_Delaunay_h */