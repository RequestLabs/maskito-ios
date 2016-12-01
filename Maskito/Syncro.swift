//
//  Singleton.swift
//  FaceWarpApp
//

import Foundation

enum CaptureType {
    case VIDEO, IMAGE
}

class Syncro {
    var warp: WarpType = WarpType.NONE
    var capturing : Bool = false
    var capture_type : CaptureType = CaptureType.VIDEO
    var calibrating : Bool = false
    var videoURL: String = ""
    
}
