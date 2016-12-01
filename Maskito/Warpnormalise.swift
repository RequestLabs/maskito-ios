//
//  Warpnormalise.swift
//

import Foundation


func boundArray(array : [Double], withUpper upper : Double, andLower lower : Double) -> [Double] {
    return array.map {
        return min(upper, max(lower, $0))
    }
}

let PRETTY_KEY = "phi.warp.pretty"
let HANDSOME_KEY = "phi.warp.handsome"

class Warper {
    var prettyObs = 0
    var _prettyScale : [Double] = [1.05, 1.05, 0.93, 0.93, 1.05, 1.05]
    var handsomeObs = 0
    var _handsomeScale : [Double] = [1.03, 1.03, 0.96, 0.96, 1.03, 1.02]
    var prettyScale : [Double] {
        get {
            return _prettyScale
        }
        set(newScale) {
            _prettyScale = boundArray(newScale, withUpper: 1.2, andLower: 0.8)
            let pretty_data = NSData(bytes: &_prettyScale, length: 6 * sizeof(Double))
            NSUserDefaults().setObject(pretty_data, forKey: PRETTY_KEY)
        }
    }
    var handsomeScale : [Double] {
        get {
            return _handsomeScale
        }
        set(newScale) {
            _handsomeScale = boundArray(newScale, withUpper: 1.2, andLower: 0.8)
            let handsome_data = NSData(bytes: &_prettyScale, length: 6 * sizeof(Double))
            NSUserDefaults().setObject(handsome_data, forKey: HANDSOME_KEY)
        }
    }
    
    init () {
        let pretty_data = NSUserDefaults().dataForKey(PRETTY_KEY)
        let handsome_data = NSUserDefaults().dataForKey(HANDSOME_KEY)
        if let pretty_data = pretty_data {
            var tmpScale : [Double] = Array(count: 6, repeatedValue: 1.0)
            pretty_data.getBytes(&tmpScale, length: 4 * sizeof(Double))
            prettyScale = tmpScale
        }
        if let handsome_data = handsome_data {
            var tmpScale : [Double] = Array(count: 6, repeatedValue: 1.0)
            handsome_data.getBytes(&tmpScale, length: 4 * sizeof(Double))
            handsomeScale = tmpScale
        }
    }

    
    var face_log : [Face] = []
    
    func doWarp (landmarks : [PhiPoint], warp : WarpType) -> ([PhiPoint], Float64) {
        // Check if we've seen this before
        let idx = findBestFace(landmarks)
        switch warp {
        case .PRETTY:
            return doAttractiveWarpPretty(landmarks, initParam: &face_log[idx].parameters)
        case .HANDSOME:
            return doAttractiveWarpHandsome(landmarks, initParam: &face_log[idx].parameters)
        case .SILLY:
            return doSillyWarp(landmarks, initParam: &face_log[idx].parameters)
        case .DYNAMIC:
            return doDynamicWarp(landmarks, initParam: &face_log[idx].parameters)
        case .TINY:
            return doTinyFaceWarp(landmarks, initParam: &face_log[idx].parameters)
        case .BIGLIPS:
            return doBigLipsWarp(landmarks, initParam: &face_log[idx].parameters)
        case .NOSE:
            return doBigNoseWarp(landmarks, initParam: &face_log[idx].parameters)
        case .NONE:
            return (landmarks, 0.0)
        case _:
            return (landmarks, 0.0)
        }
    }
    
    func doTrump(landmark1: [[PhiPoint]], landmark2: [[PhiPoint]]) ->  ([[PhiPoint]], [Float64]) {
            let all_landmarks = [landmark1[0],landmark2[0]]
            let num_faces = all_landmarks.count
            var factr : [Float64] = Array(count: num_faces, repeatedValue: 0.0)
        
        var tmp_faces = all_landmarks
        for idx in 0..<(num_faces+1)/2 {
            let idx1 = idx*2
            let idx2 = (idx1 + 1) % num_faces
            var face1 = all_landmarks[idx1]
            var face2 = all_landmarks[idx2]
            
            
            
            if idx1 + 1  >=  num_faces{
                face1 = all_landmarks[idx1]
                face2 = tmp_faces[idx2]
            }
            
            let pidx1 = findBestFace(face1)
            let pidx2 = findBestFace(face2)
            
            
            //            let (warped_faces, factr1, factr2) = doSwap(face1, landmarks2: face2, initParam1: &face_log[pidx1].parameters, initParam2: &face_log[pidx2].parameters)
            let (warped_faces, _, factr2) = doSwap(face1, landmarks2: face2, initParam1: &face_log[pidx1].parameters, initParam2: &face_log[pidx2].parameters)
            
            
            _ = Array(warped_faces[0..<68])
            let warped2 = Array(warped_faces[68..<136])
            
            //tmp_faces[idx1] = warped1
            tmp_faces[idx2] = warped2
            //factr[idx1] = factr1
            factr[idx2] = factr2
        }
        return(tmp_faces, factr)
        
    }
    
    func doSwitchFace2D(all_landmarks : [[PhiPoint]]) -> ([[PhiPoint]], [Float64]) {
        print("doSwitchFace")
        let num_faces = all_landmarks.count
        var factr : [Float64] = Array(count: num_faces, repeatedValue: 0.0)
        
        var tmp_faces = all_landmarks
        for idx in 0..<(num_faces+1)/2 {
            let idx1 = idx*2
            let idx2 = (idx1 + 1) % num_faces
            var face1 = all_landmarks[idx1]
            var face2 = all_landmarks[idx2]
            

            
            if idx1 + 1  >=  num_faces{
                face1 = all_landmarks[idx1]
                face2 = tmp_faces[idx2]
            }
        
            let pidx1 = findBestFace(face1)
            let pidx2 = findBestFace(face2)
            
            
//            let (warped_faces, factr1, factr2) = doSwap(face1, landmarks2: face2, initParam1: &face_log[pidx1].parameters, initParam2: &face_log[pidx2].parameters)
            let (warped_faces, factr1, factr2) = doShitSwap(face1, landmarks2: face2, initParam1: &face_log[pidx1].parameters, initParam2: &face_log[pidx2].parameters)
            let warped1 = Array(warped_faces[0..<68])
            let warped2 = Array(warped_faces[68..<136])

            tmp_faces[idx1] = warped1
            tmp_faces[idx2] = warped2
            factr[idx1] = factr1
            factr[idx2] = factr2
        }
        return(tmp_faces, factr)
    }
    
    func doSwitchFace3D(all_landmarks : [[PhiPoint]]) -> ([[PhiPoint]], [Float64]) {
        print("doSwitchFace")
        let num_faces = all_landmarks.count
        var factr : [Float64] = Array(count: num_faces, repeatedValue: 0.0)
        
        var tmp_faces = all_landmarks
        for idx in 0..<(num_faces+1)/2 {
            let idx1 = idx * 2
            let idx2 = (idx1 + 1) % num_faces
            let face1 = all_landmarks[idx1]
            let face2 = all_landmarks[idx2]
            
            let pidx1 = findBestFace(face1)
            let pidx2 = findBestFace(face2)
            
            
            let (warped_faces, factr1, factr2) = doSwap(face1, landmarks2: face2, initParam1: &face_log[pidx1].parameters, initParam2: &face_log[pidx2].parameters)
//            let (warped_faces, factr1, factr2) = doShitSwap(face1, landmarks2: face2, initParam1: &face_log[pidx1].parameters, initParam2: &face_log[pidx2].parameters)
            let warped1 = Array(warped_faces[0..<68])
            let warped2 = Array(warped_faces[68..<136])
            tmp_faces[idx1] = warped1
            tmp_faces[idx2] = warped2
            factr[idx1] = factr1
            factr[idx2] = factr2
        }
        return(tmp_faces, factr)
    }
    

    func doPuppetFace3D(all_landmarks : [[PhiPoint]]) -> ([[PhiPoint]], [Float64]) {
        let num_faces = all_landmarks.count
        var factr : [Float64] = Array(count: num_faces, repeatedValue: 0.0)
        
        var tmp_faces = all_landmarks

        for idx in 0..<(num_faces+1)/2 {
            let idx1 = idx * 2
            let idx2 = (idx1 + 1) % num_faces
            let face1 = all_landmarks[idx1]
            let face2 = all_landmarks[idx2]
            
            let pidx1 = findBestFace(face1)
            let pidx2 = findBestFace(face2)
            
            
            let (warped_faces, factr1, factr2) = doSwap(face1, landmarks2: face2, initParam1: &face_log[pidx1].parameters, initParam2: &face_log[pidx2].parameters)
            //            let (warped_faces, factr1, factr2) = doShitSwap(face1, landmarks2: face2, initParam1: &face_log[pidx1].parameters, initParam2: &face_log[pidx2].parameters)
            let warped1 = Array(warped_faces[0..<68])
            let warped2 = Array(warped_faces[68..<136])
            tmp_faces[idx1] = warped1

            tmp_faces[idx2] = warped2
            factr[idx1] = factr1
            factr[idx2] = factr2
        }
        return(tmp_faces, factr)
    }
    
    func doShitSwap( landmarks1 : [PhiPoint], landmarks2 : [PhiPoint], inout initParam1 : [CDouble],  inout initParam2 : [CDouble]) -> ([PhiPoint], Float64, Float64) {
        print("doShitswap")
        var concat = landmarks2
        concat.appendContentsOf(landmarks1)
        return (concat, 0.0, 0.0)
    }
    
    func findBestFace(landmarks : [PhiPoint]) -> Int {
        //prune facelibrary - if it's more than 10second old assume it's stale
        var temp_face_log : [Face] = []
        let now : NSDate = NSDate()
        for face in face_log {
            if now.timeIntervalSinceDate(face.time) < 10 {
                temp_face_log.append(face)
            }
        }
        face_log = temp_face_log
        //calculate sum of squared distances from our face library
        var distances : [Float] = []
        for face in face_log {
            var dist : Float = 0.0
            for (p_now, p_log) in zip(landmarks, face.landmarks) {
                dist += sqrt(pow(Float(p_now.x - p_log.x), 2) + pow(Float(p_now.y - p_log.y), 2))
            }
            distances.append(dist)
        }
        // Find the minimum and the index of the minimum
        let (idx, min) = distances.enumerate().reduce((Int(-1), Float(100000000.0))) {
            $0.1 < $1.1 ? $0 : $1
        }
        if min > 1000.0 {
            if face_log.count > 5 {
                face_log.removeFirst()
            }
            face_log.append(Face(landmarks: landmarks, parameters: [0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0], time: NSDate()))
            return face_log.count - 1
        } else {
            return idx
        }
    }
    
    func doAttractiveWarp(landmarks : [PhiPoint], inout initParam : [CDouble]) -> ([PhiPoint], Float64) {
        var factr : Float64 = 0
        var landmarks = landmarks
        let ans = attractive_adjusted_warp(&landmarks, &initParam, &factr);
        var safeAns : [PhiPoint] = [];
        for idx in 0..<landmarks.count {
            safeAns.append((ans[Int(idx)]))
        }
        free(ans)

        return (safeAns, factr)
    }
    
    func doAttractiveWarp2(landmarks : [PhiPoint], inout initParam : [CDouble]) -> ([PhiPoint], Float64) {
        var factr : Float64 = 0
        var landmarks = landmarks
        let ans = attractive_adjusted_warp(&landmarks, &initParam, &factr);
        var safeAns : [PhiPoint] = [];
        for idx in 0..<landmarks.count {
            safeAns.append((ans[Int(idx)]))
        }
        free(ans)
        
        return (safeAns, factr)
    }
    
    func doAttractiveWarpPretty(landmarks : [PhiPoint], inout initParam : [CDouble]) -> ([PhiPoint], Float64) {
        var factr : Float64 = 0
        var landmarks = landmarks
//        let ans = golden_inner_pretty(&landmarks, &initParam, &factr)
        let ans = apply_golden_inner_pretty(&landmarks, &initParam, &factr, &prettyScale)
        var safeAns : [PhiPoint] = []
        for idx in 0..<landmarks.count {
            safeAns.append((ans[Int(idx)]))
        }
        free(ans)
        
        return (safeAns, factr)
    }
    
    func doAttractiveWarpHandsome(landmarks : [PhiPoint], inout initParam : [CDouble]) -> ([PhiPoint], Float64) {
        var factr : Float64 = 0
        var landmarks = landmarks
//        let ans = golden_inner_handsome(&landmarks, &initParam, &factr)
        let ans = apply_golden_inner_handsome(&landmarks, &initParam, &factr, &handsomeScale)
        var safeAns : [PhiPoint] = [];
        for idx in 0..<landmarks.count {
            safeAns.append((ans[Int(idx)]))
        }
        free(ans)
        
        return (safeAns, factr)
    }
    
    func resetAttractiveWarpPretty() {
        _prettyScale = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
        prettyObs = 0
    }
    
    func addAttractiveWarpPrettyObservation(landmarks : [PhiPoint]) {
        let landmarks = landmarks
        let idx = findBestFace(landmarks)
        prettyObs += 1
        let rez = calcAttractiveWarpPretty(landmarks, initParam: &face_log[idx].parameters)
        _prettyScale = zip(_prettyScale, rez).map { return $0 + $1 }
    }
    
    func finaliseAttractiveWarpPretty() {
        _prettyScale = _prettyScale.map { $0 / Double(prettyObs) }
        prettyScale = _prettyScale
        print("New pretty scale \(prettyScale)")
    }
    
    func resetAttractiveWarpHandsome() {
        _handsomeScale = [1.0, 1.0, 1.0, 1.0, 1.0, 1.0]
        handsomeObs = 0
    }
    
    func addAttractiveWarpHandsomeObservation(landmarks : [PhiPoint]) {
        let idx = findBestFace(landmarks)
        handsomeObs += 1
        let rez = calcAttractiveWarpHandsome(landmarks, initParam: &face_log[idx].parameters)
        _handsomeScale = zip(_handsomeScale, rez).map { return $0 + $1 }
    }
    
    func finaliseAttractiveWarpHandsome() {
        _handsomeScale = _handsomeScale.map { $0 / Double(handsomeObs) }
        handsomeScale = _handsomeScale
        print("New handsome scale \(handsomeScale)")
    }
    
    func calcAttractiveWarpHandsome(landmarks : [PhiPoint], inout initParam : [CDouble]) -> ([Double]) {
        var factr : Float64 = 0
        var landmarks = landmarks
        var scaling : [Double] = Array(count: 6, repeatedValue: 1.0)
        calc_golden_inner_handsome(&landmarks, &initParam, &factr, &scaling);
        return scaling
    }
    
    func calcAttractiveWarpPretty(landmarks : [PhiPoint], inout initParam : [CDouble]) -> ([Double]) {
        var factr : Float64 = 0
        var landmarks = landmarks
        var scaling : [Double] = Array(count: 6, repeatedValue: 1.0)
        calc_golden_inner_pretty(&landmarks, &initParam, &factr, &scaling);
        return scaling
    }

    func doSillyWarp(landmarks : [PhiPoint], inout initParam : [CDouble]) -> ([PhiPoint], Float64) {
        var factr : Float64 = 0
        var landmarks = landmarks
        let ans = silly_adjusted_warp(&landmarks, &initParam, &factr);
        var safeAns : [PhiPoint] = [];
        for idx in 0..<landmarks.count {
            safeAns.append((ans[Int(idx)]))
        }
        free(ans)
        return (safeAns, factr)
    }
    
    func doBigNoseWarp(landmarks : [PhiPoint], inout initParam : [CDouble]) -> ([PhiPoint], Float64) {
        var factr : Float64 = 0
        var landmarks = landmarks
        let ans = bignose_adjusted_warp(&landmarks, &initParam, &factr);
        var safeAns : [PhiPoint] = [];
        for idx in 0..<landmarks.count {
            safeAns.append((ans[Int(idx)]))
        }
        free(ans)
        return (safeAns, factr)
    }
    
    
    func doDynamicWarp(landmarks : [PhiPoint], inout initParam : [CDouble]) -> ([PhiPoint], Float64) {
        var factr : Float64 = 0
        var landmarks = landmarks
        let ans = dynamic_adjusted_warp(&landmarks, &initParam, &factr);
        var safeAns : [PhiPoint] = [];
        for idx in 0..<landmarks.count {
            safeAns.append((ans[Int(idx)]))
        }
        free(ans)
        return (safeAns, 0.0)
    }
    
    func doBigLipsWarp(landmarks : [PhiPoint], inout initParam : [CDouble]) -> ([PhiPoint], Float64)  {
        var factr : Float64 = 0
        var landmarks = landmarks
        let ans = biglips_adjusted_warp(&landmarks, &initParam, &factr);
        var safeAns : [PhiPoint] = [];
        for idx in 0..<landmarks.count {
            safeAns.append((ans[Int(idx)]))
        }
        free(ans)
        return (safeAns, 0.0)
    }
    
    func doTinyFaceWarp(landmarks : [PhiPoint], inout initParam : [CDouble]) -> ([PhiPoint], Float64) {
        var factr : Float64 = 0
        var landmarks = landmarks
        let ans = tiny_face_warp(&landmarks, &initParam, &factr);
        var safeAns : [PhiPoint] = [];
        for idx in 0..<landmarks.count {
            safeAns.append((ans[Int(idx)]))
        }
        free(ans)
        return (safeAns, 0.0)
    }
    
    
    func doT1inyFaceWarp(landmarks : [PhiPoint], inout initParam : [CDouble]) -> ([PhiPoint], Float64) {
        var factr : Float64 = 0
        var landmarks = landmarks
        print("do tiny face")
        print(landmarks)
        let ans = tiny_face_warp(&landmarks, &initParam, &factr);
        var safeAns : [PhiPoint] = [];
        for idx in 0..<landmarks.count {
            safeAns.append((ans[Int(idx)]))
        }
        free(ans)
        return (safeAns, 0.0)
    }
    
    
    
    func doSwap(landmarks1 : [PhiPoint], landmarks2 : [PhiPoint], inout initParam1 : [CDouble],  inout initParam2 : [CDouble]) -> ([PhiPoint], Float64, Float64) {
        print("doswap")
        var factr1 : Float64 = 0.0
        var factr2 : Float64 = 0.0
        var landmarks1 = landmarks1
        var landmarks2 = landmarks2
        let ans = face_swap_warp(&landmarks1, &landmarks2, &initParam1, &initParam2, &factr1, &factr2)
        var safeAns : [PhiPoint] = [];
        for idx in 0..<(landmarks1.count + landmarks2.count) {
            safeAns.append((ans[Int(idx)]))
        }
        free(ans)
        return (safeAns, factr1, factr2)
    }
    
    func doPuppet(landmarks1 : [PhiPoint], landmarks2 : [PhiPoint], inout initParam1 : [CDouble],  inout initParam2 : [CDouble]) -> ([PhiPoint], Float64, Float64) {
        var factr1 : Float64 = 0.0
        var factr2 : Float64 = 0.0
        var landmarks1 = landmarks1
        var landmarks2 = landmarks2
        //        var param1 : [Float64]
        let ans = face_puppet_warp(&landmarks1, &landmarks2, &initParam1, &initParam2, &factr1, &factr2)
        var safeAns : [PhiPoint] = [];
        for idx in 0..<(landmarks1.count + landmarks2.count) {
            safeAns.append((ans[Int(idx)]))
        }
        free(ans)
        return (safeAns, factr1, factr2)
    }
}
