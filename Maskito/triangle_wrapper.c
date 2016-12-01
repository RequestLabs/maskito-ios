

#define SINGLE

#ifdef SINGLE
#define REAL float
#else /* not SINGLE */
#define REAL double
#endif /* not SINGLE */

#include <stdio.h>
#include <stdlib.h>
#include "triangle.h"
#include "PHI_C_Types.h"

/*****************************************************************************/
/*                                                                           */
/*  triangulate_wrapper()   Create and refine a mesh.                        */
/*                                                                           */
/*****************************************************************************/

unsigned int * rectify(int idx, int nFaces) {
    return 0;
}

PhiTriangle * triangulate_wrapper(const PhiPoint * edgesLandMarks, int nEdges, int nFaces, int * nTris)
{
    struct triangulateio in, mid, vorout;
    const int dlib_face_outline[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 26,25,24,23,22,21,20,19,18,17,0};
    int infaceTri[] = {0, 36, 17,36, 18, 17,36, 37, 18,37, 19, 18,37, 38, 19,38, 20, 19,38, 39, 20,39, 21, 20,36, 41, 37,41, 40, 37,40, 38, 37,40, 39, 38,39, 27, 21,27, 22, 21,27, 42, 22,42, 23, 22,42, 43, 23,43, 24, 23,43, 44, 24,44, 25, 24,44, 45, 25,45, 26, 25,45, 16, 26,42, 47, 43,47, 44, 43,47, 46, 44,46, 45, 44,39, 28, 27,28, 42, 27,32, 33, 30,33, 34, 30,31, 30, 32,31, 30, 29,34, 35, 30,35, 29, 30,35, 28, 29,31, 29, 28, 0,  1, 36,39, 31, 28,35, 42, 28,15, 16, 45,40, 31, 39,35, 47, 42, 1, 41, 36, 1, 40, 41,15, 45, 46,15, 46, 47,35, 15, 47, 1, 31, 40, 1,  2, 31,35, 14, 15, 2, 48, 31, 3, 48,  2, 4, 48,  3,54, 14, 35,54, 13, 14,12, 13, 54, 4,  5, 48, 5, 59, 48,11, 12, 54,55, 11, 54,10, 11, 55,56, 10, 55, 9, 10, 56, 5,  6, 59, 6, 58, 59, 6,  7, 58, 7, 57, 58, 7,  8, 57,57,  9, 56, 8,  9, 57,48, 49, 31,53, 54, 35,49, 50, 31,52, 53, 35,50, 32, 31,52, 35, 34,50, 51, 32,51, 52, 34,51, 34, 33,51, 33, 32,48, 60, 49,59, 60, 48,60, 67, 61,64, 54, 53,55, 54, 64,65, 64, 63,67, 62, 61,65, 63, 62,67, 66, 62,66, 65, 62,51, 52, 63,61, 62, 51,60, 61, 49,61, 50, 49,63, 64, 53,63, 53, 52,61, 51, 50,51, 62, 63,59, 67, 60,59, 58, 67,58, 57, 67,57, 66, 67,57, 65, 66,57, 56, 65,65, 55, 56,55, 64, 65};
    
    /* Define input points. */
    
    in.numberofpoints = nFaces * 68 + nEdges;
    in.numberofpointattributes = 0;
    in.pointlist = (REAL *) malloc(in.numberofpoints * 2 * sizeof(REAL));
    int i, fac;
    
    int * edgesLandMarksInt = (int*) edgesLandMarks;
    for (i = 0; i < in.numberofpoints * 2; i++)
    {
        
        in.pointlist[i] = (REAL)edgesLandMarksInt[i];
    }
    
    
    // segments are a list of pairs of points -
    // [p0, p1], [p1, p2], [p2, p3]....
    in.numberofsegments = nFaces * 27;
    in.segmentlist = (int *) malloc(in.numberofsegments*2 * sizeof(int));
    
    for (fac = 0; fac < nFaces; fac++)
    {
        for (i = 0; i < 27; i++)
        {
            in.segmentlist[fac*27*2 + i * 2 + 0] = dlib_face_outline[i + 0] + fac * 68;
            in.segmentlist[fac*27*2 + i * 2 + 1] = dlib_face_outline[i + 1] + fac * 68;
        }
    }
    
    in.pointmarkerlist = (int*)NULL;
    in.segmentmarkerlist = (int*)NULL;
    
    in.numberofholes = nFaces;
    in.holelist =(REAL*)malloc(in.numberofholes * 2 * sizeof(REAL));
    for (int hidx = 0; hidx < in.numberofholes; ++hidx) {
        PhiPoint facePoint = edgesLandMarks[hidx * 68 + 33]; // nose!
        REAL x = facePoint.x;
        REAL y = facePoint.y;
        in.holelist[hidx * 2 + 0] = x;
        in.holelist[hidx * 2 + 1] = y;
    }
    
    
    in.numberofregions = 0;
    
    
    /* Make necessary initializations so that Triangle can return a */
    /*   triangulation in `mid' and a voronoi diagram in `vorout'.  */
    
    mid.pointlist = (REAL *) NULL;            /* Not needed if -N switch used. */
    /* Not needed if -N switch used or number of point attributes is zero: */
    mid.pointattributelist = (REAL *) NULL;
    mid.pointmarkerlist = (int *) NULL; /* Not needed if -N or -B switch used. */
    mid.trianglelist = (int *) NULL;          /* Not needed if -E switch used. */
    /* Not needed if -E switch used or number of triangle attributes is zero: */
    mid.triangleattributelist = (REAL *) NULL;
    mid.neighborlist = (int *) NULL;         /* Needed only if -n switch used. */
    /* Needed only if segments are output (-p or -c) and -P not used: */
    mid.segmentlist = (int *) NULL;
    /* Needed only if segments are output (-p or -c) and -P and -B not used: */
    mid.segmentmarkerlist = (int *) NULL;
    mid.edgelist = (int *) NULL;             /* Needed only if -e switch used. */
    mid.edgemarkerlist = (int *) NULL;   /* Needed if -e used and -B not used. */
    
    vorout.pointlist = (REAL *) NULL;        /* Needed only if -v switch used. */
    /* Needed only if -v switch used and number of attributes is not zero: */
    vorout.pointattributelist = (REAL *) NULL;
    vorout.edgelist = (int *) NULL;          /* Needed only if -v switch used. */
    vorout.normlist = (REAL *) NULL;         /* Needed only if -v switch used. */
    
    triangulate("pczQB", &in, &mid, &vorout);
    
    PhiTriangle * outTris = (PhiTriangle *) malloc(sizeof(PhiTriangle) * (mid.numberoftriangles + nFaces * 107));
//    for (int idx = 0; idx < mid.numberoftriangles; ++idx) {
//        PhiTriangle tri;
//        tri.p0 = mid.trianglelist[idx * 3 + 0];
//        tri.p1 = mid.trianglelist[idx * 3 + 1];
//        tri.p2 = mid.trianglelist[idx * 3 + 2];
//        outTris[idx] = tri;
//    }
//    int offset = mid.numberoftriangles;
    int offset = 0;
    for (int fidx = 0; fidx < nFaces; ++fidx) {
        int inneroffset = offset + fidx * 107;
        for (int tridx = 0; tridx < 107; ++tridx ) {
            PhiTriangle tri;
            tri.p0 = infaceTri[tridx * 3 + 0] + fidx * 68;
            tri.p1 = infaceTri[tridx * 3 + 1] + fidx * 68;
            tri.p2 = infaceTri[tridx * 3 + 2] + fidx * 68;
            outTris[inneroffset + tridx] = tri;
        }
    }
    
    free(in.pointlist);
    free(in.segmentlist);
    free(in.holelist);
    free(mid.pointlist);
    free(mid.pointattributelist);
    free(mid.pointmarkerlist);
    free(mid.trianglelist);
    free(mid.triangleattributelist);
    //    free(mid.trianglearealist);
    free(mid.neighborlist);
    free(mid.segmentlist);
    free(mid.segmentmarkerlist);
    free(mid.edgelist);
    free(mid.edgemarkerlist);
    free(vorout.pointlist);
    free(vorout.pointattributelist);
    free(vorout.edgelist);
    free(vorout.normlist);
    
    *nTris = nFaces * 107;// + mid.numberoftriangles ;
    
    return outTris;
}
