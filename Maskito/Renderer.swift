//
//  Renderer.swift
//

import Foundation
import AVFoundation
import CoreVideo
import AssetsLibrary
import UIKit
import AVFoundation
import VideoToolbox

extension Array {
    func size () -> Int {
        if self.count > 0 {
            return self.count * sizeofValue(self[0])
        } else {
            return 0
        }
    }
}

extension Int32 {
    func __conversion() -> GLenum {
        return GLuint(self)
    }
    
    func __conversion() -> GLboolean {
        return GLboolean(UInt8(self))
    }
}

extension Int {
    func __conversion() -> Int32 {
        return Int32(self)
    }
    
    func __conversion() -> GLubyte {
        return GLubyte(self)
    }
    
}

func dist(p1 : PhiPoint, _ p2 : PhiPoint) -> Float {
    let xdist = p1.x - p2.x
    let ydist = p1.y - p2.y
    let sqdist = xdist * xdist + ydist * ydist
    return sqrt(Float(sqdist))
}

func extremaOfPixelBuffer(pb : CVPixelBufferRef) -> (Float, Float) {
    let width = CVPixelBufferGetWidth(pb)
    let height = CVPixelBufferGetHeight(pb)
    let rowbytes = CVPixelBufferGetBytesPerRow(pb)
    CVPixelBufferLockBaseAddress(pb, kCVPixelBufferLock_ReadOnly)
    defer {
        CVPixelBufferUnlockBaseAddress(pb, kCVPixelBufferLock_ReadOnly)
    }
    
    let ptr = UnsafeMutablePointer<UInt8>(CVPixelBufferGetBaseAddress(pb))
    
    var minval = Float.infinity
    var maxval = -Float.infinity
    
    for r in 0..<height {
        for c in 0..<width {
            let idx = r * rowbytes + c
            let r = Float(ptr[idx + 0])
            let g = Float(ptr[idx + 1])
            let b = Float(ptr[idx + 2])
            let M = max(r, max(g, b))
            let m = min(r, min(g, b))
            let C = (M - m)
            if C != 0 {
                let v = (r + b + g) / 3.0
                let s = C / v
                minval = s < minval ? s : minval
                maxval = s > maxval ? s : maxval
            }
        }
    }
    return (minval, maxval)
}

class Renderer : NSObject, AVCaptureVideoDataOutputSampleBufferDelegate, AVCaptureAudioDataOutputSampleBufferDelegate {
    
    var pauseRenderer = false
    
    var calibrateTime : NSDate? = nil
    var failCalibrateTime : NSDate? = nil
    
    var camera : Int //Default camera int---0 means back camera, 1 means front
    var pastCamera : Int
    
    var assetWriter : AVAssetWriter? = nil
    var awAudio : AVAssetWriterInput? = nil
    var awVideo : AVAssetWriterInput? =  nil
    var assetWriterPixelBufferInput : AVAssetWriterInputPixelBufferAdaptor? = nil
    var videoStartTime = NSDate()
    var vidLoc : NSURL?
    var isWriting : Bool = false
    
    let context : EAGLContext
    let layer : CAEAGLLayer
    
    var doFaceBlur : Bool = true
    var captureNext : Bool = false
    
    
    
    var recorder = Recorder()
    
    var textureManager : TextureManager?
    var shaderManager : ShaderManager?
    var vertexManager : VertexManager?
    let faceDetector : FaceFinder = FaceFinder()
    let warper : Warper = Warper()
    
    var delegate : AppDelegate? = nil
    var warpType : WarpType = .PRETTY
    
    let scale = 4 // how much we shrink small image by
    let toothThreshold : GLfloat = 0.3
    
    var orientation : UIDeviceOrientation = .Portrait
    var pastOrientation : UIDeviceOrientation = .Portrait
    
    var hasInitedAW : Bool = false
    
    var cTime = CMTime()
    
    
    init(withContext c: EAGLContext, andLayer l: CAEAGLLayer, andCamera k: Int) {
        context = c
        layer = l
        camera = k
        pastCamera = k
        super.init()
        self.delegate = UIApplication.sharedApplication().delegate as? AppDelegate
        self.textureManager = TextureManager(withContext: context, andLayer: layer)
        self.shaderManager = ShaderManager()!
        self.vertexManager = VertexManager()

        

        // loadRoboFaceTexture()
        // loadMoFaceTexture()
    }
    
    func destroyTexture() {
        textureManager!.destroy()
    }

    func captureOutput(captureOutput : AVCaptureOutput, didOutputSampleBuffer sampleBuffer: CMSampleBufferRef,
                    fromConnection connection: AVCaptureConnection)
    {
        if let _ = captureOutput as? AVCaptureAudioDataOutput {
            if recorder.state == .Recording {
                recorder.addAudioSampleBuffer(sampleBuffer)
            }
        } else if let _ = captureOutput as? AVCaptureVideoDataOutput {
            self.textureManager?.loadTextureFromSampleBuffer(sampleBuffer)
            dispatch_async(dispatch_get_main_queue(), {
                let time = CMSampleBufferGetPresentationTimeStamp(sampleBuffer)
                self.cTime = time
                if self.textureManager != nil {
                    self.render(time)
                }
            })
        }
    }
        
    func setupForOrientation(withScale scale : Int = 2) {
        
        let vwidth = CVPixelBufferGetWidth(textureManager!.videoPixelBuffer!)
        let vheight = CVPixelBufferGetHeight(textureManager!.videoPixelBuffer!)
        
        let width = UIScreen.mainScreen().bounds.width * UIScreen.mainScreen().scale
        let height = UIScreen.mainScreen().bounds.height * UIScreen.mainScreen().scale

        let vaspect = Float(vwidth) / Float(vheight)
        let saspect = Float(width) / Float(height)
        
        if camera != pastCamera {
            pastOrientation = .Unknown
        }
        
        pastCamera = camera
        if !delegate!.syncro.capturing {
            switch camera {
                case 1:
                  switch orientation {
                  
                    case pastOrientation:
                        break
                    case .LandscapeLeft:
                        vertexManager!.fillPreprocessVBO(forFlip: .VERTICAL, andRotate90: false)
                        vertexManager!.fillPostprocessVBO(forFlip: .BOTH, andRotate90: true, forVideoAspect: vaspect, andScreenAspect: saspect)
                        textureManager!.makeUprightPixelBuffer(withWidth: vwidth, andHeight: vheight)
                        textureManager!.makeOutputPixelBuffer(withWidth: vwidth, andHeight: vheight)
                        textureManager!.makeSmallerPixelBuffer(withWidth: vwidth, andHeight: vheight, andScale: scale)
                        textureManager!.makeHBlurTexture(withWidth: vwidth, andHeight: vheight, andScale: scale)
                        textureManager!.makeVBlurTexture(withWidth: vwidth, andHeight: vheight, andScale: scale)
                    case .LandscapeRight:
                        vertexManager!.fillPreprocessVBO(forFlip: .HORIZONTAL, andRotate90: false)
                        vertexManager!.fillPostprocessVBO(forFlip: .NONE, andRotate90: true, forVideoAspect: vaspect, andScreenAspect: saspect)
                        textureManager!.makeUprightPixelBuffer(withWidth: vwidth, andHeight: vheight)
                        textureManager!.makeOutputPixelBuffer(withWidth: vwidth, andHeight: vheight)
                        textureManager!.makeSmallerPixelBuffer(withWidth: vwidth, andHeight: vheight, andScale: scale)
                        textureManager!.makeHBlurTexture(withWidth: vwidth, andHeight: vheight, andScale: scale)
                        textureManager!.makeVBlurTexture(withWidth: vwidth, andHeight: vheight, andScale: scale)
                    case .PortraitUpsideDown:
                        vertexManager!.fillPreprocessVBO(forFlip: .BOTH, andRotate90: true)
                        vertexManager!.fillPostprocessVBO(forFlip: .HORIZONTAL, andRotate90: false, forVideoAspect: vaspect, andScreenAspect: saspect)
                        textureManager!.makeUprightPixelBuffer(withWidth: vheight, andHeight: vwidth)
                        textureManager!.makeOutputPixelBuffer(withWidth: vheight, andHeight: vwidth)
                        textureManager!.makeSmallerPixelBuffer(withWidth: vheight, andHeight: vwidth, andScale: scale)
                        textureManager!.makeHBlurTexture(withWidth: vheight, andHeight: vwidth, andScale: scale)
                        textureManager!.makeVBlurTexture(withWidth: vwidth, andHeight: vheight, andScale: scale)
                    case _:
                        vertexManager!.fillPreprocessVBO(forFlip: .NONE, andRotate90: true)
                        vertexManager!.fillPostprocessVBO(forFlip: .VERTICAL, andRotate90: false, forVideoAspect: vaspect, andScreenAspect: saspect)
                        textureManager!.makeUprightPixelBuffer(withWidth: vheight, andHeight: vwidth)
                        textureManager!.makeOutputPixelBuffer(withWidth: vheight, andHeight: vwidth)
                        textureManager!.makeSmallerPixelBuffer(withWidth: vheight, andHeight: vwidth, andScale: scale)
                        textureManager!.makeHBlurTexture(withWidth: vheight, andHeight: vwidth, andScale: scale)
                        textureManager!.makeVBlurTexture(withWidth: vwidth, andHeight: vheight, andScale: scale)
                    }
                case 0:
                  switch orientation{
                    case pastOrientation:
                        break
                    case .LandscapeLeft:
                        vertexManager!.fillPreprocessVBO(forFlip: .NONE, andRotate90: false)
                        vertexManager!.fillPostprocessVBO(forFlip: .BOTH, andRotate90: true, forVideoAspect: vaspect, andScreenAspect: saspect)
                        textureManager!.makeUprightPixelBuffer(withWidth: vwidth, andHeight: vheight)
                        textureManager!.makeOutputPixelBuffer(withWidth: vwidth, andHeight: vheight)
                        textureManager!.makeSmallerPixelBuffer(withWidth: vwidth, andHeight: vheight, andScale: scale)
                        textureManager!.makeHBlurTexture(withWidth: vwidth, andHeight: vheight, andScale: scale)
                        textureManager!.makeVBlurTexture(withWidth: vwidth, andHeight: vheight, andScale: scale)
                    case .LandscapeRight:
                        vertexManager!.fillPreprocessVBO(forFlip: .BOTH, andRotate90: false)
                        vertexManager!.fillPostprocessVBO(forFlip: .NONE, andRotate90: true, forVideoAspect: vaspect, andScreenAspect: saspect)
                        textureManager!.makeUprightPixelBuffer(withWidth: vwidth, andHeight: vheight)
                        textureManager!.makeOutputPixelBuffer(withWidth: vwidth, andHeight: vheight)
                        textureManager!.makeSmallerPixelBuffer(withWidth: vwidth, andHeight: vheight, andScale: scale)
                        textureManager!.makeHBlurTexture(withWidth: vwidth, andHeight: vheight, andScale: scale)
                        textureManager!.makeVBlurTexture(withWidth: vwidth, andHeight: vheight, andScale: scale)
                    case .PortraitUpsideDown:
                        vertexManager!.fillPreprocessVBO(forFlip: .HORIZONTAL, andRotate90: true)
                        vertexManager!.fillPostprocessVBO(forFlip: .HORIZONTAL, andRotate90: false, forVideoAspect: vaspect, andScreenAspect: saspect)
                        textureManager!.makeUprightPixelBuffer(withWidth: vheight, andHeight: vwidth)
                        textureManager!.makeOutputPixelBuffer(withWidth: vheight, andHeight: vwidth)
                        textureManager!.makeSmallerPixelBuffer(withWidth: vheight, andHeight: vwidth, andScale: scale)
                        textureManager!.makeHBlurTexture(withWidth: vheight, andHeight: vwidth, andScale: scale)
                        textureManager!.makeVBlurTexture(withWidth: vwidth, andHeight: vheight, andScale: scale)
                      case _:
                        vertexManager!.fillPreprocessVBO(forFlip: .VERTICAL, andRotate90: true)
                        vertexManager!.fillPostprocessVBO(forFlip: .VERTICAL, andRotate90: false, forVideoAspect: vaspect, andScreenAspect: saspect)
                        textureManager!.makeUprightPixelBuffer(withWidth: vheight, andHeight: vwidth)
                        textureManager!.makeOutputPixelBuffer(withWidth: vheight, andHeight: vwidth)
                        textureManager!.makeSmallerPixelBuffer(withWidth: vheight, andHeight: vwidth, andScale: scale)
                        textureManager!.makeHBlurTexture(withWidth: vheight, andHeight: vwidth, andScale: scale)
                        textureManager!.makeVBlurTexture(withWidth: vwidth, andHeight: vheight, andScale: scale)
                }
                case _:
                  switch orientation{
                    case pastOrientation:
                        break
                    case .LandscapeLeft:
                        vertexManager!.fillPreprocessVBO(forFlip: .HORIZONTAL, andRotate90: false)
                        vertexManager!.fillPostprocessVBO(forFlip: .VERTICAL, andRotate90: false, forVideoAspect: vaspect, andScreenAspect: saspect)
                        textureManager!.makeUprightPixelBuffer(withWidth: vwidth, andHeight: vheight)
                        textureManager!.makeOutputPixelBuffer(withWidth: vwidth, andHeight: vheight)
                        textureManager!.makeSmallerPixelBuffer(withWidth: vwidth, andHeight: vheight, andScale: scale)
                        textureManager!.makeHBlurTexture(withWidth: vwidth, andHeight: vheight, andScale: scale)
                        textureManager!.makeVBlurTexture(withWidth: vwidth, andHeight: vheight, andScale: scale)
                    case .LandscapeRight:
                        vertexManager!.fillPreprocessVBO(forFlip: .VERTICAL, andRotate90: false)
                        vertexManager!.fillPostprocessVBO(forFlip: .VERTICAL, andRotate90: false, forVideoAspect: vaspect, andScreenAspect: saspect)
                        textureManager!.makeUprightPixelBuffer(withWidth: vwidth, andHeight: vheight)
                        textureManager!.makeOutputPixelBuffer(withWidth: vwidth, andHeight: vheight)
                        textureManager!.makeSmallerPixelBuffer(withWidth: vwidth, andHeight: vheight, andScale: scale)
                        textureManager!.makeHBlurTexture(withWidth: vwidth, andHeight: vheight, andScale: scale)
                        textureManager!.makeVBlurTexture(withWidth: vwidth, andHeight: vheight, andScale: scale)
                    case .Portrait:
                        vertexManager!.fillPreprocessVBO(forFlip: .VERTICAL, andRotate90: true)
                        vertexManager!.fillPostprocessVBO(forFlip: .BOTH, andRotate90: false, forVideoAspect: vaspect, andScreenAspect: saspect)
                        textureManager!.makeUprightPixelBuffer(withWidth: vheight, andHeight: vwidth)
                        textureManager!.makeOutputPixelBuffer(withWidth: vheight, andHeight: vwidth)
                        textureManager!.makeSmallerPixelBuffer(withWidth: vheight, andHeight: vwidth, andScale: scale)
                        textureManager!.makeHBlurTexture(withWidth: vheight, andHeight: vwidth, andScale: scale)
                        textureManager!.makeVBlurTexture(withWidth: vwidth, andHeight: vheight, andScale: scale)
                    case .PortraitUpsideDown:
                        vertexManager!.fillPreprocessVBO(forFlip: .BOTH, andRotate90: true)
                        vertexManager!.fillPostprocessVBO(forFlip: .VERTICAL, andRotate90: false, forVideoAspect: vaspect, andScreenAspect: saspect)
                        textureManager!.makeUprightPixelBuffer(withWidth: vheight, andHeight: vwidth)
                        textureManager!.makeOutputPixelBuffer(withWidth: vheight, andHeight: vwidth)
                        textureManager!.makeSmallerPixelBuffer(withWidth: vheight, andHeight: vwidth, andScale: scale)
                        textureManager!.makeHBlurTexture(withWidth: vheight, andHeight: vwidth, andScale: scale)
                        textureManager!.makeVBlurTexture(withWidth: vwidth, andHeight: vheight, andScale: scale)
                    case _:
                        vertexManager!.fillPreprocessVBO(forFlip: .NONE, andRotate90: false)
                        vertexManager!.fillPostprocessVBO(forFlip: .NONE, andRotate90: false, forVideoAspect: vaspect, andScreenAspect: saspect)
                        textureManager!.makeUprightPixelBuffer(withWidth: vwidth, andHeight: vheight)
                        textureManager!.makeOutputPixelBuffer(withWidth: vwidth, andHeight: vheight)
                        textureManager!.makeSmallerPixelBuffer(withWidth: vwidth, andHeight: vheight, andScale: scale)
                        textureManager!.makeHBlurTexture(withWidth: vwidth, andHeight: vheight, andScale: scale)
                        textureManager!.makeVBlurTexture(withWidth: vwidth, andHeight: vheight, andScale: scale)
                    }
            
            }
        }
    }

    func preprocessRender() {
        let (xyzSlot, uvSlot, alphaSlot, textureSlot) = self.shaderManager!.activatePassThroughShader()
        let (num, type) = vertexManager!.bindPreprocessVBO(withPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
        textureManager!.bindVideoTextureToSlot(textureSlot)
        
        // Render real size, upright
        textureManager?.bindUprightTextureAsOutput()
        textureManager!.setViewPortForUprightTexture()
        glDrawElements(GLenum(GL_TRIANGLES), num, type, nil)
        
        downSizeBlurFilter()
        
        vertexManager!.unbindPreprocessVBO(fromPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
    }
    
    func downSizeBlurFilter() {
        
        var (xyzSlot, uvSlot, alphaSlot, textureSlot) = shaderManager!.activateHAvgShader(withScale: 0.0001)
        var (num, type) = vertexManager!.bindPassVBO(withPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
        
        textureManager!.bindUprightTextureToSlot(textureSlot)
        textureManager!.bindHBlurTextureAsOutput()
        textureManager!.setViewPortForHBlurTexture()
        glDrawElements(GLenum(GL_TRIANGLES), num, type, nil)
        vertexManager!.unbindPassVBO(fromPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
        
        (xyzSlot, uvSlot, alphaSlot, textureSlot) = shaderManager!.activateVAvgShader(withScale: 0.0001)
        (num, type) = vertexManager!.bindPassVBO(withPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
        
        textureManager!.bindHBlurTextureToSlot(textureSlot)
        textureManager!.bindSmallerTextureAsOutput()
        textureManager!.setViewPortForSmallerTexture()
        glDrawElements(GLenum(GL_TRIANGLES), num, type, nil)
        vertexManager!.unbindPassVBO(fromPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
        
        
    }
    
    func hblurRender() {
        let (xyzSlot, uvSlot, alphaSlot, textureSlot) = shaderManager!.activateHBlurShader(withScale: 1.0)
        let (num, type) = vertexManager!.bindPassVBO(withPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
        
        textureManager!.bindSmallerTextureToSlot(textureSlot)
        textureManager!.bindHBlurTextureAsOutput()
        textureManager!.setViewPortForHBlurTexture()
        glDrawElements(GLenum(GL_TRIANGLES), num, type, nil)
        vertexManager!.unbindPassVBO(fromPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
        
    }
    
    func vblurRender() {
        let (xyzSlot, uvSlot, alphaSlot, textureSlot) = shaderManager!.activateVBlurShader(withScale: 1.0)
        let (num, type) = vertexManager!.bindPassVBO(withPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
        
        textureManager!.bindHBlurTextureToSlot(textureSlot)
        textureManager!.bindVBlurTextureAsOutput()
        textureManager!.setViewPortForVBlurTexture()
        glDrawElements(GLenum(GL_TRIANGLES), num, type, nil)
        vertexManager!.unbindPassVBO(fromPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
    }
    
    func blurRender() {
        hblurRender()
        vblurRender()
    }
    
    func blurToOutput() {
        let (xyzSlot, uvSlot, alphaSlot, textureSlot) = shaderManager!.activateVBlurShader(withScale: 1.0)
        let (num, type) = vertexManager!.bindPassVBO(withPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
        textureManager!.bindVBlurTextureToSlot(textureSlot)
        textureManager!.bindOutputTextureAsOutput()
        textureManager!.setViewPortForOutputTexture()
        glDrawElements(GLenum(GL_TRIANGLES), num, type, nil)
        vertexManager!.unbindPassVBO(fromPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
    }
    
    func clearToOutput() {
        let (xyzSlot, uvSlot, alphaSlot, textureSlot) = shaderManager!.activatePassThroughShader()
        let (num, type) = vertexManager!.bindPassVBO(withPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
        textureManager!.bindUprightTextureToSlot(textureSlot)
        textureManager!.bindOutputTextureAsOutput()
        textureManager!.setViewPortForOutputTexture()
        glDrawElements(GLenum(GL_TRIANGLES), num, type, nil)
        vertexManager!.unbindPassVBO(fromPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
    }
    
    private func pixelBufferFromCGImage(image: CGImageRef) -> CVPixelBufferRef {
        let size = UIImage(CGImage: image).size
        let options: Dictionary = [ kCVPixelBufferCGImageCompatibilityKey as String : NSNumber(bool: true), kCVPixelBufferCGBitmapContextCompatibilityKey as String : NSNumber(bool: true) ]
        var pxbuffer: CVPixelBufferRef?
        let _ = CVPixelBufferCreate(kCFAllocatorDefault, Int(size.width), Int(size.height), kCVPixelFormatType_32ARGB, options, &pxbuffer)
        
        CVPixelBufferLockBaseAddress(pxbuffer!, 0)
        let pxdata = CVPixelBufferGetBaseAddress(pxbuffer!)
        let rgbColorSpace = CGColorSpaceCreateDeviceRGB()
        
        let context = CGBitmapContextCreate(pxdata, Int(size.width), Int(size.height), 8, 4 * Int(size.width), rgbColorSpace, CGImageAlphaInfo.PremultipliedFirst.rawValue)
        CGContextConcatCTM(context, CGAffineTransformMakeRotation(0))
        CGContextDrawImage(context, CGRectMake(0, 0, CGFloat(size.width), CGFloat(size.height)), image)
        
        CVPixelBufferUnlockBaseAddress(pxbuffer!, 0);
        
        return pxbuffer!
    }
    

    
    func findFaces() {
        // Functions return a 'tidyUp' closure which we call to release the pixel buffer
        var facePoints : [[NSValue]] = []
        if let big = textureManager!.uprightPixelBuffer, let small = textureManager!.smallPixelBuffer {
            facePoints = faceDetector.facesPointsInBigImage(big, andSmallImage: small, withScale: Int32(scale)) as! [[NSValue]]
        }
        let numFaces = facePoints.count
        guard numFaces > 0 else {
            return
        }
        let facePhiPoints = facePoints.map {
            (perFace) -> [PhiPoint] in
            return perFace.map {
                (value) -> PhiPoint in
                return value.PhiPointValue
            }
        }
        

        switch warpType {
        case .NONE:
            break
        case .ROBOT:
            for pointArray in facePoints {
                let uvpoints = pointArray.map {
                    return $0.PhiPointValue
                }
                //self.textureManager?.loadMoFaceTexture()
                //drawRobotEye(UV: uvpoints)
                //drawRobotFace(UV: uvpoints)

                drawMoFace(UV: uvpoints)
            }
        case .MASK:
            for pointArray in facePoints {
                let uvpoints = pointArray.map {
                    return $0.PhiPointValue
                }
                drawMaskFace(UV: uvpoints)
            }
        case .REDNOSE:
            for pointArray in facePoints {
                let uvpoints = pointArray.map {
                    return $0.PhiPointValue
                }
                //self.textureManager?.loadMoFaceTexture()
                //drawRobotEye(UV: uvpoints)
                //drawRobotFace(UV: uvpoints)
                drawRNFace(UV: uvpoints)
            }
        case .GASMASK:
            for pointArray in facePoints {
                let uvpoints = pointArray.map {
                    return $0.PhiPointValue
                }
                drawGasmask(UV: uvpoints)
            }
        case .FACIALTATTOO:
            for pointArray in facePoints {
                let uvpoints = pointArray.map {
                    return $0.PhiPointValue
                }
                drawFaceTattoo(UV: uvpoints)
            }
        case .MOUSTACHE:
            for pointArray in facePoints {
                let uvpoints = pointArray.map {
                    return $0.PhiPointValue
                }
                drawMoFace(UV: uvpoints)
            }
        case .SWAP:
            let (xyzArray, factrs) = warper.doSwitchFace2D(facePhiPoints)
            for (uvPoints, (xyPoints, _)) in zip(facePhiPoints, zip(xyzArray, factrs)) {
                drawClearFace(XY: xyPoints, UV: uvPoints, withAlphas: (1.0, 0.0, 1.0, 1.0))
                drawRightEye(XY: xyPoints, UV: uvPoints)
                drawLeftEye(XY: xyPoints, UV: uvPoints)
                drawMouth(XY: xyPoints, UV: uvPoints)
                drawInnerMouth(XY: xyPoints, UV: xyPoints)
            }
        case .PUPPET:

            let (xyzArray, factrs) = warper.doPuppetFace3D(facePhiPoints)
            for (uvPoints, (xyPoints, _)) in zip(xyzArray, zip(facePhiPoints, factrs)) {
                drawClearFace(XY: xyPoints, UV: uvPoints, withAlphas: (1.0, 0.0, 1.0, 1.0))
                drawRightEye(XY: xyPoints, UV: uvPoints)
                drawLeftEye(XY: xyPoints, UV: uvPoints)
                drawMouth(XY: xyPoints, UV: uvPoints)
//                drawInnerMouth(XY: uvPoints, UV: uvPoints)
            }
        case .HANDSOME:
            for pointArray in facePoints {
                let uvPoints = pointArray.map {
                    return $0.PhiPointValue
                }
                
                let (xyPoints, rotationAmount) = doWarp(uvPoints)
                drawBlurFace(XY: xyPoints, UV: uvPoints, withRotation: Float(rotationAmount))
                drawClearFace(XY: xyPoints, UV: uvPoints, withAlphas: (0.7, 1.0, 0.9, 0.9))
                drawRightEye(XY: xyPoints, UV: uvPoints)
                drawLeftEye(XY: xyPoints, UV: uvPoints)
                drawMouth(XY: xyPoints, UV: uvPoints)
                let (ratio, min, max) = prepTeeth(UVs: uvPoints)
                drawBrighterMouth(XY: xyPoints, UV: uvPoints, withMin: min, andMax: max, andRatio: ratio, andRotation: Float(rotationAmount))
            }
        case .PRETTY:
            for pointArray in facePoints {
                let uvPoints = pointArray.map {
                    return $0.PhiPointValue
                }
                
                let (xyPoints, rotationAmount) = doWarp(uvPoints)
                drawBlurFace(XY: xyPoints, UV: uvPoints, withRotation: Float(rotationAmount))
                drawClearFace(XY: xyPoints, UV: uvPoints, withAlphas: (0.6, 1.0, 0.9, 0.9))
                drawRightEye(XY: xyPoints, UV: uvPoints)
                drawLeftEye(XY: xyPoints, UV: uvPoints)
                drawMouth(XY: xyPoints, UV: uvPoints)
                let (ratio, min, max) = prepTeeth(UVs: uvPoints)
                drawBrighterMouth(XY: xyPoints, UV: uvPoints, withMin: min, andMax: max, andRatio: ratio, andRotation: Float(rotationAmount))
            }
        case .SILLY:
            for pointArray in facePoints {
                let uvPoints = pointArray.map {
                    return $0.PhiPointValue
                }
                let (xyPoints, rotationAmount) = doWarp(uvPoints)
                drawClearFace(XY: xyPoints, UV: uvPoints, withAlphas: (1.0, 1.0, 1.0, 1.0))
                drawRightEye(XY: xyPoints, UV: uvPoints)
                drawLeftEye(XY: xyPoints, UV: uvPoints)
                drawMouth(XY: xyPoints, UV: uvPoints)
                let (ratio, min, max) = prepTeeth(UVs: uvPoints)

                drawBrighterMouth(XY: xyPoints, UV: uvPoints, withMin: min, andMax: max, andRatio: ratio, andRotation: Float(rotationAmount))
            }
        case _:
            for pointArray in facePoints {
                let uvPoints = pointArray.map {
                    return $0.PhiPointValue
                }
                let (xyPoints, _) = doWarp(uvPoints)
                drawClearFace(XY: xyPoints, UV: uvPoints, withAlphas: (1.0, 1.0, 1.0, 1.0))
                drawRightEye(XY: xyPoints, UV: uvPoints)
                drawLeftEye(XY: xyPoints, UV: uvPoints)
                drawMouth(XY: xyPoints, UV: uvPoints)
            }
        }
        
    }
    
    func calibrateFaces() {
        print("calibrateFaces")
        // Functions return a 'tidyUp' closure which we call to release the pixel buffer
        var facePoints : [[NSValue]] = []
        if let big = textureManager!.uprightPixelBuffer, let small = textureManager!.smallPixelBuffer {
            facePoints = faceDetector.facesPointsInBigImage(big, andSmallImage: small, withScale: Int32(scale)) as! [[NSValue]]
        }
        let numFaces = facePoints.count
        guard numFaces > 0 else {
            return
        }
        /*
        let facePhiPoints = facePoints.map {
            (perFace) -> [PhiPoint] in
            return perFace.map {
                (value) -> PhiPoint in
                return value.PhiPointValue
            }
        }*/
        switch warpType {
        case .HANDSOME:
            for pointArray in facePoints {
                let uvPoints = pointArray.map {
                    return $0.PhiPointValue
                }
                warper.addAttractiveWarpHandsomeObservation(uvPoints)
            }
        case .PRETTY:
            for pointArray in facePoints {
                let uvPoints = pointArray.map {
                    return $0.PhiPointValue
                }
                warper.addAttractiveWarpPrettyObservation(uvPoints)
            }
        case .TINY:
            for pointArray in facePoints {
                let uvPoints = pointArray.map {
                    return $0.PhiPointValue
                }
                warper.addAttractiveWarpPrettyObservation(uvPoints)
            }
        case _:
            break
        }
        
    }

    
    func drawRobotEye(UV uvs : [PhiPoint]) {
        let box = textureManager!.uprightRect
        if let box = box {
            glEnable(GLenum(GL_BLEND))
            glBlendFuncSeparate(GLenum(GL_SRC_ALPHA), GLenum(GL_ONE_MINUS_SRC_ALPHA), GLenum(GL_ZERO), GLenum(GL_ONE));
            let (xyzSlot, uvSlot, alphaSlot, textureSlot) = shaderManager!.activateBasicShader()
            vertexManager!.fillRoboEye(UV: uvs, inBox: box)
            textureManager!.bindRoboFaceToSlot(textureSlot)
            textureManager!.bindOutputTextureAsOutput()
            textureManager!.setViewPortForOutputTexture()
            let (num, type) = vertexManager!.bindRoboEyeVBO(withPositionSlot: xyzSlot, andUVSlot: uvSlot, andBrightenSlot: alphaSlot)
            glDrawElements(GLenum(GL_TRIANGLES), num, type, nil)
            vertexManager?.unbindRoboEyeVBO(fromPositionSlot: xyzSlot, andUVSlot: uvSlot, andBrightenSlot: alphaSlot)
            glDisable(GLenum(GL_BLEND))
        }
    }
    
    func drawRobotFace(UV uvs : [PhiPoint]) {
        let box = textureManager!.uprightRect
        if let box = box {
            glEnable(GLenum(GL_BLEND))
            glBlendFuncSeparate(GLenum(GL_SRC_ALPHA), GLenum(GL_ONE_MINUS_SRC_ALPHA), GLenum(GL_ZERO), GLenum(GL_ONE));
            let (xyzSlot, uvSlot, alphaSlot, textureSlot) = shaderManager!.activateBasicShader()
            vertexManager!.fillRoboFace(UV: uvs, inBox: box)
            textureManager!.bindRoboFaceToSlot(textureSlot)
            textureManager!.bindOutputTextureAsOutput()
            textureManager!.setViewPortForOutputTexture()
            let (num, type) = vertexManager!.bindRoboFaceVBO(withPositionSlot: xyzSlot, andUVSlot: uvSlot, andBrightenSlot: alphaSlot)
            glDrawElements(GLenum(GL_TRIANGLES), num, type, nil)
            vertexManager?.unbindRoboFaceVBO(fromPositionSlot: xyzSlot, andUVSlot: uvSlot, andBrightenSlot: alphaSlot)
            glDisable(GLenum(GL_BLEND))
        }
    }
    
    func drawGasmask(UV uvs : [PhiPoint]) {
        let box = textureManager!.uprightRect
        if let box = box {
            glEnable(GLenum(GL_BLEND))
            glBlendFuncSeparate(GLenum(GL_SRC_ALPHA), GLenum(GL_ONE_MINUS_SRC_ALPHA), GLenum(GL_ZERO), GLenum(GL_ONE));
            let (xyzSlot, uvSlot, alphaSlot, textureSlot) = shaderManager!.activateBasicShader()
            vertexManager!.fillRoboFace(UV: uvs, inBox: box)
            textureManager?.bindGasmaskToSlot(textureSlot)
            textureManager!.bindOutputTextureAsOutput()
            textureManager!.setViewPortForOutputTexture()
            let (num, type) = vertexManager!.bindRoboFaceVBO(withPositionSlot: xyzSlot, andUVSlot: uvSlot, andBrightenSlot: alphaSlot)
            glDrawElements(GLenum(GL_TRIANGLES), num, type, nil)
            vertexManager?.unbindRoboFaceVBO(fromPositionSlot: xyzSlot, andUVSlot: uvSlot, andBrightenSlot: alphaSlot)
            glDisable(GLenum(GL_BLEND))
        }
    }
    
    func drawFaceTattoo(UV uvs : [PhiPoint]) {
        let box = textureManager!.uprightRect
        if let box = box {
            glEnable(GLenum(GL_BLEND))
            glBlendFuncSeparate(GLenum(GL_SRC_ALPHA), GLenum(GL_ONE_MINUS_SRC_ALPHA), GLenum(GL_ZERO), GLenum(GL_ONE));
            let (xyzSlot, uvSlot, alphaSlot, textureSlot) = shaderManager!.activateBasicShader()
            vertexManager!.fillRoboFace(UV: uvs, inBox: box)
            textureManager?.bindFacialTattooToSlot(textureSlot)
            //            textureManager?.bindMoFaceToSlot(textureSlot)
            textureManager!.bindOutputTextureAsOutput()
            textureManager!.setViewPortForOutputTexture()
            let (num, type) = vertexManager!.bindRoboFaceVBO(withPositionSlot: xyzSlot, andUVSlot: uvSlot, andBrightenSlot: alphaSlot)
            glDrawElements(GLenum(GL_TRIANGLES), num, type, nil)
            vertexManager?.unbindRoboFaceVBO(fromPositionSlot: xyzSlot, andUVSlot: uvSlot, andBrightenSlot: alphaSlot)
            glDisable(GLenum(GL_BLEND))
        }
    }
    
    func drawMoFace(UV uvs : [PhiPoint]) {
        let box = textureManager!.uprightRect
        if let box = box {
            glEnable(GLenum(GL_BLEND))
            glBlendFuncSeparate(GLenum(GL_SRC_ALPHA), GLenum(GL_ONE_MINUS_SRC_ALPHA), GLenum(GL_ZERO), GLenum(GL_ONE));
            let (xyzSlot, uvSlot, alphaSlot, textureSlot) = shaderManager!.activateBasicShader()
            vertexManager!.fillRoboFace(UV: uvs, inBox: box)
            textureManager?.bindMoFaceToSlot(textureSlot)
//            textureManager?.bindMoFaceToSlot(textureSlot)
            textureManager!.bindOutputTextureAsOutput()
            textureManager!.setViewPortForOutputTexture()
            let (num, type) = vertexManager!.bindRoboFaceVBO(withPositionSlot: xyzSlot, andUVSlot: uvSlot, andBrightenSlot: alphaSlot)
            glDrawElements(GLenum(GL_TRIANGLES), num, type, nil)
            vertexManager?.unbindRoboFaceVBO(fromPositionSlot: xyzSlot, andUVSlot: uvSlot, andBrightenSlot: alphaSlot)
            glDisable(GLenum(GL_BLEND))
        }
    }

    func drawRNFace(UV uvs : [PhiPoint]) {
        let box = textureManager!.uprightRect
        if let box = box {
            glEnable(GLenum(GL_BLEND))
            glBlendFuncSeparate(GLenum(GL_SRC_ALPHA), GLenum(GL_ONE_MINUS_SRC_ALPHA), GLenum(GL_ZERO), GLenum(GL_ONE));
            let (xyzSlot, uvSlot, alphaSlot, textureSlot) = shaderManager!.activateBasicShader()
            vertexManager!.fillRoboFace(UV: uvs, inBox: box)
            //textureManager?.bindRoboFaceToSlot(textureSlot)
            //textureManager?.bindMoFaceToSlot(textureSlot)
            textureManager?.bindRedNoseToSlot(textureSlot)
            textureManager!.bindOutputTextureAsOutput()
            textureManager!.setViewPortForOutputTexture()
            let (num, type) = vertexManager!.bindRoboFaceVBO(withPositionSlot: xyzSlot, andUVSlot: uvSlot, andBrightenSlot: alphaSlot)
            glDrawElements(GLenum(GL_TRIANGLES), num, type, nil)
            vertexManager?.unbindRoboFaceVBO(fromPositionSlot: xyzSlot, andUVSlot: uvSlot, andBrightenSlot: alphaSlot)
            glDisable(GLenum(GL_BLEND))
        }
    }
    
    func drawMaskFace(UV uvs : [PhiPoint]) {
        let box = textureManager!.uprightRect
        if let box = box {
            glEnable(GLenum(GL_BLEND))
            glBlendFuncSeparate(GLenum(GL_SRC_ALPHA), GLenum(GL_ONE_MINUS_SRC_ALPHA), GLenum(GL_ZERO), GLenum(GL_ONE));
            let (xyzSlot, uvSlot, alphaSlot, textureSlot) = shaderManager!.activateBasicShader()
            vertexManager!.fillRoboFace(UV: uvs, inBox: box)
            textureManager?.bindMaskFaceToSlot(textureSlot)
            textureManager!.bindOutputTextureAsOutput()
            textureManager!.setViewPortForOutputTexture()
            let (num, type) = vertexManager!.bindRoboFaceVBO(withPositionSlot: xyzSlot, andUVSlot: uvSlot, andBrightenSlot: alphaSlot)
            glDrawElements(GLenum(GL_TRIANGLES), num, type, nil)
            vertexManager?.unbindRoboFaceVBO(fromPositionSlot: xyzSlot, andUVSlot: uvSlot, andBrightenSlot: alphaSlot)
            glDisable(GLenum(GL_BLEND))
        }
    }

    
    func prepTeeth(UVs uvs : [PhiPoint]) -> (GLfloat, GLfloat, GLfloat) {
        let width = dist(uvs[60], uvs[64])
        let height = dist(uvs[66], uvs[62])
        let ratio = height / width
        return (ratio, 0, 0)
    }
    
    func drawBlurFace(XY xy: [PhiPoint], UV uv: [PhiPoint], withRotation rotation : Float) {
        let box = textureManager!.uprightRect
        if let box = box {
            glEnable(GLenum(GL_BLEND))
            glBlendFuncSeparate(GLenum(GL_ONE), GLenum(GL_ONE_MINUS_SRC_ALPHA), GLenum(GL_ZERO), GLenum(GL_ONE))
            let (xyzSlot, uvSlot, alphaSlot, textureSlot) = shaderManager!.activatePassThroughShader()
            // Fade blur in as rotation changes from 0.65 to 0.75
            let scaler = min(max(((rotation - 0.65) / 0.1), 0.0), 1.0)
            vertexManager!.fillFaceVertex(XY: xy, UV: uv, inBox: box, inFaceAlpha: scaler, outFaceAlpha: scaler, aroundEyesAlpha: scaler, aroundMouthAlpha: scaler)
            vertexManager!.selectFacePart(FacePart.SKIN)
            let (num, type) = vertexManager!.bindFaceVBO(withPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
            textureManager!.bindVBlurTextureToSlot(textureSlot)
            textureManager!.bindOutputTextureAsOutput()
            textureManager!.setViewPortForOutputTexture()
            glDrawElements(GLenum(GL_TRIANGLES), num, type, nil)
            vertexManager!.unbindFaceVBO(fromPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
            glDisable(GLenum(GL_BLEND))
        }
    }
    
    func drawClearFace(XY xy: [PhiPoint], UV uv: [PhiPoint], withAlphas alpha : (Float, Float, Float, Float)) {
        let box = textureManager!.uprightRect
        if let box = box {
            glEnable(GLenum(GL_BLEND))
            glBlendFuncSeparate(GLenum(GL_SRC_ALPHA), GLenum(GL_ONE_MINUS_SRC_ALPHA), GLenum(GL_ZERO), GLenum(GL_ONE))
            let (xyzSlot, uvSlot, alphaSlot, textureSlot) = shaderManager!.activatePassThroughShader()
            vertexManager!.fillFaceVertex(XY: xy, UV: uv, inBox: box, inFaceAlpha: alpha.0, outFaceAlpha: alpha.1, aroundEyesAlpha: alpha.2, aroundMouthAlpha: alpha.3)
            vertexManager!.selectFacePart(FacePart.SKIN)
            let (num, type) = vertexManager!.bindFaceVBO(withPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
            textureManager!.bindUprightTextureToSlot(textureSlot)
            textureManager!.bindOutputTextureAsOutput()
            textureManager!.setViewPortForOutputTexture()
            glDrawElements(GLenum(GL_TRIANGLES), num, type, nil)
            vertexManager!.unbindFaceVBO(fromPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
            glDisable(GLenum(GL_BLEND))
        }
    }
    
    func drawMouth(XY xy: [PhiPoint], UV uv: [PhiPoint]) {
        let box = textureManager!.uprightRect
        if let box = box {
            
            let (xyzSlot, uvSlot, alphaSlot, textureSlot) = shaderManager!.activatePassThroughShader()
            vertexManager!.fillFaceVertex(XY: xy, UV: uv, inBox: box)
            vertexManager!.selectFacePart(FacePart.ALL_MOUTH)
            let (num, type) = vertexManager!.bindFaceVBO(withPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
            textureManager!.bindUprightTextureToSlot(textureSlot)
            textureManager!.bindOutputTextureAsOutput()
            textureManager!.setViewPortForOutputTexture()
            glDrawElements(GLenum(GL_TRIANGLES), num, type, nil)
            vertexManager!.unbindFaceVBO(fromPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
            
        }
    }
    
    func drawInnerMouth(XY xy: [PhiPoint], UV uv: [PhiPoint]) {
        let box = textureManager!.uprightRect
        if let box = box {
            glEnable(GLenum(GL_BLEND))
            glBlendFuncSeparate(GLenum(GL_SRC_ALPHA), GLenum(GL_ONE_MINUS_SRC_ALPHA), GLenum(GL_ZERO), GLenum(GL_ONE))
            let (xyzSlot, uvSlot, alphaSlot, textureSlot) = shaderManager!.activatePassThroughShader()
            vertexManager!.fillFadeMouthVBO(UV: uv, XY: xy, inBox: box)
            let (num, type) = vertexManager!.bindFadeMouthVBO(withPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
            textureManager!.bindUprightTextureToSlot(textureSlot)
            textureManager!.bindOutputTextureAsOutput()
            textureManager!.setViewPortForOutputTexture()
            glDrawElements(GLenum(GL_TRIANGLES), num, type, nil)
            vertexManager!.unbindFadeMouthVBO(fromPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
            glDisable(GLenum(GL_BLEND))
        }
    }
    
    func drawBrighterMouth(XY xy: [PhiPoint], UV uv: [PhiPoint], withMin mn: GLfloat, andMax mx : GLfloat, andRatio ratio : GLfloat, andRotation rotation: Float) {
        let box = textureManager!.uprightRect
        if let box = box {
            let uvSlice = Array(uv[48..<68])
            let xySlice = Array(xy[48..<68])
            
            let scaler = min(max(((rotation - 0.6) / 0.2), 0.0), 1.0)
            
            let (xyzSlot, uvSlot, brightenSlot, textureSlot) = shaderManager!.activateDentistShader(withMinimum: 0, andMaximum: 0, andThreshold: 0)
            vertexManager!.fillBrighterMouthVBO(UV: uvSlice, XY: xySlice, inBox: box, withBrightness:  scaler)
            let (num, type) = vertexManager!.bindBrighterMouthVBO(withPositionSlot: xyzSlot, andUVSlot: uvSlot, andBrightenSlot: brightenSlot)
            textureManager!.bindUprightTextureToSlot(textureSlot)
            textureManager!.bindOutputTextureAsOutput()
            textureManager!.setViewPortForOutputTexture()
            glDrawElements(GLenum(GL_TRIANGLES), num, type, nil)
            vertexManager!.unbindBrighterMouthVBO(fromPositionSlot: xyzSlot, andUVSlot: uvSlot, andBrightenSlot: brightenSlot)
        }
    }
    
    func drawRightEye(XY xy: [PhiPoint], UV uv: [PhiPoint]) {
        let box = textureManager!.uprightRect
        if let box = box {
            let (xyzSlot, uvSlot, alphaSlot, textureSlot) = shaderManager!.activatePassThroughShader()
            vertexManager!.fillFaceVertex(XY: xy, UV: uv, inBox: box)
            vertexManager!.selectFacePart(FacePart.RIGHT_EYE)
            let (num, type) = vertexManager!.bindFaceVBO(withPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
            textureManager!.bindUprightTextureToSlot(textureSlot)
            textureManager!.bindOutputTextureAsOutput()
            textureManager!.setViewPortForOutputTexture()
            glDrawElements(GLenum(GL_TRIANGLES), num, type, nil)
            vertexManager!.unbindFaceVBO(fromPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
        }
    }
    func drawLeftEye(XY xy: [PhiPoint], UV uv: [PhiPoint]) {
        let box = textureManager!.uprightRect
        if let box = box {
            let (xyzSlot, uvSlot, alphaSlot, textureSlot) = shaderManager!.activatePassThroughShader()
            vertexManager!.fillFaceVertex(XY: xy, UV: uv, inBox: box)
            vertexManager!.selectFacePart(FacePart.LEFT_EYE)
            let (num, type) = vertexManager!.bindFaceVBO(withPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
            textureManager!.bindUprightTextureToSlot(textureSlot)
            textureManager!.bindOutputTextureAsOutput()
            textureManager!.setViewPortForOutputTexture()
            glDrawElements(GLenum(GL_TRIANGLES), num, type, nil)
            vertexManager!.unbindFaceVBO(fromPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
        }
    }
    
    func doWarp(uv : [PhiPoint]) -> ([PhiPoint], Float64) {
        return warper.doWarp(uv, warp: warpType)
    }
    
    func scheduleSave() {
        captureNext = true
    }
    
    func renderToScreen() {
        let (xyzSlot, uvSlot, alphaSlot, textureSlot) = shaderManager!.activatePassThroughShader()
        
        textureManager!.bindScreen()
        
        let width = UIScreen.mainScreen().bounds.width * UIScreen.mainScreen().scale
        let height = UIScreen.mainScreen().bounds.height * UIScreen.mainScreen().scale
        
        switch orientation {
        case .LandscapeLeft:
            glViewport(0, 0, GLsizei(width), GLsizei(height))

        case .LandscapeRight:
            glViewport(0, 0, GLsizei(width), GLsizei(height))

        case .Portrait:
            glViewport(0, 0, GLsizei(width), GLsizei(height))
        case .PortraitUpsideDown:
            glViewport(0, 0, GLsizei(width), GLsizei(height))
        case _:
            glViewport(0, 0, GLsizei(width), GLsizei(height))
        }
        
        let (num, type) = vertexManager!.bindPostprocessVBO(withPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
        textureManager!.bindOutputTextureToSlot(textureSlot)
        glClear(GLenum(GL_COLOR_BUFFER_BIT))
        //print(GL_TRIANGLES)
        glDrawElements(GLenum(GL_TRIANGLES), num, type, nil)
        vertexManager!.unbindPostprocessVBO(fromPositionSlot: xyzSlot, andUVSlot: uvSlot, andAlphaSlot: alphaSlot)
    }
    
    func render(initTime: CMTime) {
        let _pastOrientation = orientation
        warpType = (self.delegate?.syncro.warp)!
        orientation = UIDeviceOrientation.Unknown
        setupForOrientation(withScale: scale)
        //print(warpType)
        self.preprocessRender() // Generates upright and small-upright images
        self.blurRender() // Render from small upright texture to hblurred texture to vlburred to output texture
        
        if !delegate!.syncro.calibrating {
            clearToOutput() // renders upright to output
            findFaces() // Finds faces and renders them to output
            renderToScreen()
            pastOrientation = _pastOrientation
            if delegate!.syncro.capturing {
                if delegate!.syncro.capture_type == .IMAGE {
                    self.context.presentRenderbuffer(Int(GL_RENDERBUFFER))
                    delegate!.syncro.capturing = false
                    textureManager!.saveOutput(camera == 1) // camera == 1 is true if using front camera!
                } else {
                    if recorder.state == .Idle {
                        //print("recording state idle")
                        let width = CVPixelBufferGetWidth(textureManager!.outputPixelBuffer!)
                        let height = CVPixelBufferGetHeight(textureManager!.outputPixelBuffer!)
                        recorder.prepareRecord(forWidth: width, andHeight: height)
                        delegate!.disableUI()
                        videoStartTime = NSDate()
                    }  else if recorder.state == .Recording {
                       
                        let intVal = Int(ceil(NSDate().timeIntervalSinceDate(videoStartTime)))
                        delegate!.setRecordTime(intVal)
                        

                        
                        if let pb = textureManager?.outputPixelBuffer {
                            recorder.addVideoFrame(pb, atTime: initTime)
                        }
                        

                    }
                }
            } else {
                //print("recording")
                if recorder.state == .Recording {
                    print("recorder.state Recording")
                    delegate!.setRecordTime(0)
                    print("recorder.capturing false")
                    delegate!.syncro.capturing = false
                    print("recorder stop and save")
                    
                    recorder.stopRecordingAndSave()
                    delegate!.enableUI()
                }
            }
            self.context.presentRenderbuffer(Int(GL_RENDERBUFFER))
        } else { // calibrating the pretty/handsome warp
            delegate!.hideInstructions(false) // re-draw instructions
            switch UIDevice.currentDevice().orientation {
            case .Portrait:
                if let time = calibrateTime { // do calibration
                    failCalibrateTime = nil // nil this out, since either: it's already nil or the clever user has rotated their device
                    let deltaT = NSDate().timeIntervalSinceDate(time)
                    if deltaT > 5.5 {
                        switch warpType {
                        case .PRETTY:
                            warper.finaliseAttractiveWarpPretty()
                        case .HANDSOME:
                            warper.finaliseAttractiveWarpHandsome()
                        case _:
                            break
                        }
                        delegate!.syncro.calibrating = false
                        calibrateTime = nil
                        delegate!.redrawUI()
                    }
                    else if deltaT >= 2 {
                        calibrateFaces()
                        delegate!.setTextForCount(Int(ceil(5.0 - deltaT)))
                    }
                    blurToOutput()
                    renderToScreen()
                    self.context.presentRenderbuffer(Int(GL_RENDERBUFFER))
                } else {
                    calibrateTime = NSDate()
                    switch warpType {
                    case .PRETTY:
                        warper.resetAttractiveWarpPretty()
                    case .HANDSOME:
                        warper.resetAttractiveWarpHandsome()
                    case _:
                        break
                    }
                }
            case _:
                if let time = failCalibrateTime { // display portait warning for three seconds
                    let deltaT = NSDate().timeIntervalSinceDate(time)
                    blurToOutput()
                    renderToScreen()
                    self.context.presentRenderbuffer(Int(GL_RENDERBUFFER))
                    if deltaT > 3 {
                        delegate!.syncro.calibrating = false
                        calibrateTime = nil
                        delegate!.redrawUI()
                    }
                } else {
                    failCalibrateTime = NSDate()
                }
            }
        }
    }
    
}