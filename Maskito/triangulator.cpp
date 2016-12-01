

#import <GTEngine.h>
#import "triangulator.h"
#import "Clarkson-Delaunay.h"
#include <CoreGraphics/CoreGraphics.h>
#include <stdlib.h>
#include <vector>
// factorials (multiplies example)
#include <iostream>     // std::cout
#include <algorithm>
#include <functional>   // std::multiplies
#include <numeric>      // std::partial_sum
#include <set>

std::vector<PhiTriangle> infaceTri {{ 0, 36, 17},{36, 18, 17},{36, 37, 18},{37, 19, 18},{37, 38, 19},{38, 20, 19},{38, 39, 20},{39, 21, 20},{36, 41, 37},{41, 40, 37},{40, 38, 37},{40, 39, 38},{39, 27, 21},{27, 22, 21},{27, 42, 22},{42, 23, 22},{42, 43, 23},{43, 24, 23},{43, 44, 24},{44, 25, 24},{44, 45, 25},{45, 26, 25},{45, 16, 26},{42, 47, 43},{47, 44, 43},{47, 46, 44},{46, 45, 44},{39, 28, 27},{28, 42, 27},{32, 33, 30},{33, 34, 30},{31, 30, 32},{31, 30, 29},{34, 35, 30},{35, 29, 30},{35, 28, 29},{31, 29, 28},{ 0,  1, 36},{39, 31, 28},{35, 42, 28},{15, 16, 45},{40, 31, 39},{35, 47, 42},{ 1, 41, 36},{ 1, 40, 41},{15, 45, 46},{15, 46, 47},{35, 15, 47},{ 1, 31, 40},{ 1,  2, 31},{35, 14, 15},{ 2, 48, 31},{ 3, 48,  2},{ 4, 48,  3},{54, 14, 35},{54, 13, 14},{12, 13, 54},{ 4,  5, 48},{ 5, 59, 48},{11, 12, 54},{55, 11, 54},{10, 11, 55},{56, 10, 55},{ 9, 10, 56},{ 5,  6, 59},{ 6, 58, 59},{ 6,  7, 58},{ 7, 57, 58},{ 7,  8, 57},{57,  9, 56},{ 8,  9, 57},{48, 49, 31},{53, 54, 35},{49, 50, 31},{52, 53, 35},{50, 32, 31},{52, 35, 34},{50, 51, 32},{51, 52, 34},{51, 34, 33},{51, 33, 32},{48, 60, 49},{59, 60, 48},{60, 67, 61},{64, 54, 53},{55, 54, 64},{65, 64, 63},{67, 62, 61},{65, 63, 62},{67, 66, 62},{66, 65, 62},{51, 52, 63},{61, 62, 51},{60, 61, 49},{61, 50, 49},{63, 64, 53},{63, 53, 52},{61, 51, 50},{51, 62, 63},{59, 67, 60},{59, 58, 67},{58, 57, 67},{57, 66, 67},{57, 65, 66},{57, 56, 65},{65, 55, 56},{55, 64, 65}};

PhiTriangle operator +(PhiTriangle tri, int offset) {
    return PhiTriangle{tri.p0 + offset, tri.p1 + offset, tri.p2 + offset};
}

//ConstrainedDelaunay2 del;

/*
Delaunay triangles may not include (for a given face):
 1. and point outside the face, a point on the face-surround, and a 
 */

bool allFaceIndicies(int faceLower, int faceUpper, PhiTriangle tri) {
    // faceLower : inclusive floor of face indices
    // faceUpper : exclusive ceil of face indicies
    // triangles : current list of assumed safe triangles
    bool allInFace = true; // Check if each point is equal to or above faceLower and below faceUpper. If any false, this is false
    allInFace &= ((tri.p0 >= faceLower) && (tri.p0 < faceUpper));
    allInFace &= ((tri.p1 >= faceLower) && (tri.p1 < faceUpper));
    allInFace &= ((tri.p2 >= faceLower) && (tri.p2 < faceUpper));
    return allInFace;
}

std::vector<PhiTriangle> prunedTriangles(std::vector<PhiTriangle> possibleTriangles, int numFaces) {
    std::vector<PhiTriangle> tmp;
    for (int fidx = 0; fidx < numFaces; ++fidx) {
        int offset = fidx * 68;
        tmp.clear();
        int faceLower = offset + 27;
        int faceUpper = faceLower + 68;
        for (PhiTriangle tri : possibleTriangles) {
            bool inface = allFaceIndicies(faceLower, faceUpper, tri);
            if (!inface) {
                possibleTriangles.push_back(tri);
            }
        }
        std::swap(tmp, possibleTriangles);
    }
    return possibleTriangles;
}

int check_intersection(PhiTriangle &v11, const std::vector<int> &v22){
    std::vector<int> v1((int*)&v11, ((int*)&v11)+3);
    std::vector<int> v2 = v22;
    
    std::vector<int> v;
    v.resize(v1.size() + v2.size());
    std::vector<int>::iterator it;
    
    std::sort(v1.begin(),v1.end());
    std::sort(v2.begin(),v2.end());
    
    it=std::set_intersection (v1.begin(), v1.end(), v2.begin(), v2.end(), v.begin());
    v.resize(it-v.begin());
    
    return (v.size() > 0);
};

int distict_abs(const std::vector<int>& v)
{
    std::set<int> distinct_container;
    
    for(auto curr_int = v.begin(), end = v.end(); // no need to call v.end() multiple times
        curr_int != end;
        ++curr_int)
    {
        // std::set only allows single entries
        // since that is what we want, we don't care that this fails
        // if the second (or more) of the same value is attempted to
        // be inserted.
        distinct_container.insert(abs(*curr_int));
    };
    
    return (int)distinct_container.size();
};

bool tri_el_not_in_interior(PhiTriangle &v11, const std::vector<int>& v22){
    
    std::vector<int> delaunay_el((int*)&v11, ((int*)&v11)+3);
    std::vector<int> range_of_interior_points = v22;
    
    int sum = 0;
    for (int i = 0; i < delaunay_el.size(); i++){
        if (std::find(range_of_interior_points.begin(), range_of_interior_points.end(), delaunay_el[i]) != range_of_interior_points.end()){
            sum += 1;
        };
    };
    
    bool flag = true;
    if (sum > 0){
        flag = false;
    };
    return flag;
};

// Needs c linkage to be imported to Swift
extern "C" {
PhiTriangle * unsafeTidyIndices(const PhiPoint * edgesLandMarks, int numEdges, int numFaces, int * nTris) {
    // CALLER TO FREE RETURN VALUE
    int numVertices;
    int dim = 2;
    int numPoints = numEdges + numFaces * 68;
    // Result is an numVertices array of unsigned integers, in row major form representing a matrix (numVertices / 3) x 3    
    unsigned int * unsafeResultRaw = BuildTriangleIndexList((int *)edgesLandMarks, 0, numPoints, dim, 0, &numVertices);
    if (numVertices == 0) {
        std::cout << "Triangulation failed" << std::endl;
        PhiTriangle * triResults = (PhiTriangle *)(unsafeResultRaw);
        return triResults;
    }

    else
    {
    
//    std::vector<int> idx_of_edge_points;
//    for (int i = 0; i < numEdges; i++)
//    {
//        idx_of_edge_points.push_back(i + numFaces*68);
//    }
//    
//    std::vector<int> range_of_end_points;
//    for( int i = 0; i < numFaces; i++ ){
//        range_of_end_points.push_back( ((i + 1) * 68) - 1 );
//    };
//    
//    std::vector<int> range_of_interior_points;
//    for (int face_num = 0; face_num < numFaces; face_num++){
//        for( int i = 27 + face_num * 68; i < 67 + face_num * 68; i++ ){
//            range_of_interior_points.push_back( i );
//        };
//    };
    
    // cast to become triangles...
    PhiTriangle * triResults = (PhiTriangle *)(unsafeResultRaw);
//    std::vector<PhiTriangle> possibleTriang(triResults, triResults + numVertices / 3);
//    std::vector<PhiTriangle> finalTriResults;
//    int idx = 0;
//    for (int i = 0; i < numVertices / 3; i++)
//    {
//        if (check_intersection(possibleTriang[i], idx_of_edge_points))
//        {
//            finalTriResults.push_back(possibleTriang[i]);
//            idx++;
//        }
//        else
//        {
//            std::vector<int> sums;
//            sums.resize(range_of_end_points.size());
//            std::vector<int> delaunay((int*)(triResults + i), (int*)(triResults + i + 1));
//            for (int j = 0; j < range_of_end_points.size(); j++){
//                sums[j] = 0;
//                for (int k = 0; k < 3; k++)
//                {
//                    if (range_of_end_points[j] >= delaunay[k]){
//                        sums[j] += 1;
//                    };
//                }
//            };
//            if (distict_abs(sums) > 1)
//            {
//                finalTriResults.push_back(possibleTriang[i]);
//                idx++;
//            }
//            else if (tri_el_not_in_interior(possibleTriang[i], range_of_interior_points))
//            {
//                finalTriResults.push_back(possibleTriang[i]);
//                idx++;
//            };
//        };
//    };
//
//    int offset;
//    for (int i = 0; i < numFaces; i++)
//    {
//        offset = 68*i;
//        
//        for (int j = 0; j < 107; j++)
//        {
//            finalTriResults[idx] = infaceTri[j];
//            finalTriResults[idx].p0 += offset;
//            finalTriResults[idx].p1 += offset;
//            finalTriResults[idx].p2 += offset;
//            idx++;
//        };
//    };
//    
    
    // wrap in vector
//    std::vector<PhiTriangle> possibleTriang(triResults, triResults + numVertices / 3);
//    std::cout << "Found " << numVertices / 3 << " triangles" << std::endl;
    
    // For each face, calculate the lower and upper indices for points entirely inside that face (excluding face surround).
    // Append a triangle to localGoodTriang if it doesn't have any points inside that given face.
    // Assign localGoodTriang to goodTriang and repeat for the next face.
//    for (int faceIdx = 0; faceIdx < numFaces; ++faceIdx) {
//        std::vector<PhiTriangle> localTriang;
//        int faceLower = numEdges + faceIdx * 68 + 27;      // Lower (inclusive) bound
//        int faceUpper = numEdges + faceIdx * 68 + 68; // Upper (exclusive) bound
//        for (PhiTriangle tri : possibleTriang) {
//            if (!pointInFace(tri, faceLower, faceUpper)) {
//                localTriang.push_back(tri);
//            }
//        }
//        possibleTriang = localTriang;
//    }
//    
//    std::cout << "Pruned down to " << possibleTriang.size() << " triangles" << std::endl;
//    free(unsafeResultRaw);
    
    // For each face, append the correctly incremented indexes of the inface triangulation
//    for (int faceIdx = 0; faceIdx < numFaces; ++faceIdx) {
//        int offset = faceIdx * 68 + numEdges;
//        for (PhiTriangle tri : infaceTri) {
//            possibleTriang.push_back(tri + offset);
//        }
//    }
    // Now for dirty bit.
    // Set ntris to be the size of 'tidied'
    *nTris = numVertices / 3;
    
//    *nTris = idx;
        return triResults;
//    return (PhiTriangle*)(&finalTriResults[0]);
    }
}
}

