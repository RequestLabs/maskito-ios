//
//  Triangulator.swift
//

import Foundation


func tidyIndices(edgesAndLandmarks : [PhiPoint], numEdges : Int, numFaces : Int) -> [PhiTriangle] {
    var tris : Int32 = 0
    var edgesAndLandmarks = edgesAndLandmarks
    let ans = triangulate_wrapper(&edgesAndLandmarks, Int32(numEdges), Int32(numFaces), &tris)
    var safeAns : [PhiTriangle] = []
    for idx in 0..<Int(tris) {
        safeAns.append(ans[idx])
    }
    
    free(ans)
    return safeAns
}
