//
//  CaptureManager.swift
//

import Foundation
import AVFoundation

enum CaptureError : ErrorType {
    case CONNECTION_FAILURE
}

class CaptureManager {
    
    let session : AVCaptureSession
    
    static func devices() -> [AVCaptureDevice]{
        return  AVCaptureDevice.devices() as! [AVCaptureDevice]
    }
    
    init?(withDevice videoDevice: AVCaptureDevice) {
        session = AVCaptureSession()
        
        let audioDevice = AVCaptureDevice.defaultDeviceWithMediaType(AVMediaTypeAudio) // default audio device
        
        // Attempt to attach the device to our session
        var videoInput : AVCaptureDeviceInput? = nil
        var audioInput : AVCaptureDeviceInput? = nil
        do {
            
            videoInput = try AVCaptureDeviceInput(device: videoDevice)
            audioInput = try AVCaptureDeviceInput(device: audioDevice)
        } catch {
            print("Failed to connect device \(videoDevice.description)")
            return nil
        }
        session.beginConfiguration()
        session.addInput(audioInput!)
        session.addInput(videoInput!)
        session.commitConfiguration()
    }
    
    func connectToRenderer(renderer: Renderer) throws {
        // Attempt to initiate an output, tied to the sample buffer delegate (renderer)

        let videoOutput = AVCaptureVideoDataOutput()
        let audioOutput = AVCaptureAudioDataOutput()
        
        videoOutput.alwaysDiscardsLateVideoFrames = true // Stops things getting bogged down if the CPU is being hammered
        videoOutput.videoSettings = [kCVPixelBufferPixelFormatTypeKey as String: Int(kCVPixelFormatType_32BGRA),] // 32BGRA needed to be OpenGL compatible
        videoOutput.setSampleBufferDelegate(renderer, queue: dispatch_get_main_queue()) // run on main thread
        audioOutput.setSampleBufferDelegate(renderer, queue: dispatch_get_main_queue())
        session.beginConfiguration()
        session.addOutput(videoOutput)
        session.addOutput(audioOutput)
        session.sessionPreset = AVCaptureSessionPresetiFrame1280x720
        session.commitConfiguration()
    }
    
    func start() {
        session.startRunning()
        
    }
    
    func stop() {
        session.stopRunning()
    }
    
}