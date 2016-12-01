//
//  Textures.swift
//

import Foundation
import CoreMedia
import CoreVideo
import Photos
import GLKit
import AssetsLibrary

class TextureManager {
    
    var delegate : AppDelegate? = nil
    
    let albumName = "Pixurgery"
    
    let teethHeight = 20
    let teethWidth = 50
    
    // Top level stuff
    let context : EAGLContext
    let layer : CAEAGLLayer
    var textureCache : CVOpenGLESTextureCacheRef? = nil
    
    // References to Output render and framebuffers (for screen)
    var outputRenderBuffer : GLuint = GLuint()
    var outputFrameBuffer : GLuint = GLuint()
    
    // References to input video texture, live inside TEXTURE1
    var videoTexture : CVOpenGLESTextureRef? = nil
    var videoPixelBuffer : CVPixelBufferRef? = nil
    
    // References to Rotated render and textures - re-draw camera output here - live inside TEXTURE2
    var uprightTexture : CVOpenGLESTextureRef? = nil
    var uprightPixelBuffer : CVPixelBufferRef? = nil
    
    // References to Rotated render and textures - re-draw upright buffer smaller - live inside TEXTURE3
    var smallTexture : CVOpenGLESTextureRef? = nil
    var smallPixelBuffer : CVPixelBufferRef? = nil
    
    // Reference to HBlur texture - upright, blurred - live inside TEXTURE4
    var hblurTexture : GLuint = GLuint()
    var hblurwidth : Int = 0
    var hblurheight : Int = 0
    
    // Reference to VBlur texture - upright, blurred - lives inside TEXTURE5
    var vblurTexture : GLuint = GLuint()
    var vblurwidth : Int = 0
    var vblurheight : Int = 0
    
    // Reference to Output texture - upright, everything draws to this before screenshot - lives inside TEXTURE6
    var outputTexture : CVOpenGLESTextureRef? = nil
    var outputPixelBuffer : CVPixelBufferRef? = nil
    
    // Reference to robot eye texture. Lives inside TEXTURE 7
    var eyeTexture : GLKTextureInfo? = nil
    
    // Reference to robot face texture. Lives inside TEXTURE 8
    var faceTexture : GLKTextureInfo? = nil
    var moTexture : GLKTextureInfo? = nil
    var rednoseTexture: GLKTextureInfo? = nil
    var maskTexture: GLKTextureInfo? = nil
    var facialTattooTexture : GLKTextureInfo? = nil
    var gasMaskTexture: GLKTextureInfo? = nil

    init?(withContext cntxt : EAGLContext, andLayer lyr : CAEAGLLayer) {
        delegate = UIApplication.sharedApplication().delegate as? AppDelegate
        context = cntxt
        layer = lyr
        
        // Setup a texture cache
        let cacheStatus = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, nil, self.context, nil, &textureCache)
        if cacheStatus != kCVReturnSuccess {
            print("Creating texture cache failed with error \(cacheStatus)")
            return nil
        }
        setupScreen()
        setupNormalTextures()
        //loadEyeTexture()
        //loadRoboFaceTexture()
        loadMoFaceTexture()
        loadRedNoseTexture()
        loadRedNoseTexture()
        loadGasmaskTexture()
        loadFacialTattooTexture()
        loadMaskFaceTexture()
    }
    
    var size : CGRect {
        return UIScreen.mainScreen().bounds
    }
    
    func destroy() {
        
    }
    
    // MARK: loading data from a sample buffer (video) into OpenGL
    func loadTextureFromSampleBuffer(sampleBuffer: CMSampleBuffer) { // Video texture lives in TEXTURE0
        self.videoTexture = nil
        CVOpenGLESTextureCacheFlush(self.textureCache!, 0);

        self.videoPixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer)

        let width = CVPixelBufferGetWidth(self.videoPixelBuffer!)
        let height = CVPixelBufferGetHeight(self.videoPixelBuffer!)
        
        glActiveTexture(GLenum(GL_TEXTURE1))
        let ret = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
            self.textureCache!,
            self.videoPixelBuffer!,
            nil,
            GLenum(GL_TEXTURE_2D),
            GLint(GL_RGBA),
            GLsizei(width),
            GLsizei(height),
            GLenum(GL_BGRA),
            GLenum(GL_UNSIGNED_BYTE),
            0,
            &self.videoTexture)
        if ret != kCVReturnSuccess {
            print("CVOpenGLESTextureCacheCreateTextureFromImage failed with code \(ret)")
        }
        
        glBindTexture(
            CVOpenGLESTextureGetTarget(videoTexture!),
            CVOpenGLESTextureGetName(videoTexture!)
        )

        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_S), GL_CLAMP_TO_EDGE)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_T), GL_CLAMP_TO_EDGE)
        glActiveTexture(GLenum(GL_TEXTURE0))
    }
    func bindVideoTextureToSlot(textureSlot : GLint) {
        glUniform1i(textureSlot, 1)
    }
    
    func saveOutput(flip : Bool = false) {
        print("saveOutput")
        if let pb = outputPixelBuffer {
            savePixelBuffer(pb)
        }
    }
    
    // MARK: output buffers for iOS
    func setupScreen() {
        glGenRenderbuffers(1, &outputRenderBuffer)
        glBindRenderbuffer(GLenum(GL_RENDERBUFFER), outputRenderBuffer)
        context.renderbufferStorage(Int(GL_RENDERBUFFER), fromDrawable:self.layer)
        glGenFramebuffers(1, &outputFrameBuffer)
        glBindFramebuffer(GLenum(GL_FRAMEBUFFER), outputFrameBuffer)
    }
    func bindScreen() {
        glFramebufferRenderbuffer(GLenum(GL_FRAMEBUFFER), GLenum(GL_COLOR_ATTACHMENT0), GLenum(GL_RENDERBUFFER), outputRenderBuffer)
    }
    
    // MARK: generate buffer for flipped textures
    func makeUprightPixelBuffer(withWidth width : Int, andHeight height : Int) {
        uprightPixelBuffer = nil
        uprightTexture = nil
        CVOpenGLESTextureCacheFlush(textureCache!, 0)
        let size = CGSizeMake(CGFloat(width), CGFloat(height))
        
        glActiveTexture(GLenum(GL_TEXTURE2)) // upright texture lives in texture 2
        generatePixelBuffer(&uprightPixelBuffer, andTexture: &uprightTexture, withSize: size)
        glBindTexture(CVOpenGLESTextureGetTarget(uprightTexture!), CVOpenGLESTextureGetName(uprightTexture!))
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_S), GL_CLAMP_TO_EDGE)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_T), GL_CLAMP_TO_EDGE)
        glActiveTexture(GLenum(GL_TEXTURE0))
    }
    func bindUprightTextureAsOutput() {
        if uprightTexture != nil {
            glFramebufferTexture2D(GLenum(GL_FRAMEBUFFER), GLenum(GL_COLOR_ATTACHMENT0), GLenum(GL_TEXTURE_2D), CVOpenGLESTextureGetName(uprightTexture!), 0);
        }

    }
    func bindUprightTextureToSlot(textureSlot : GLint) {
        glUniform1i(textureSlot, 2)
    }
    func setViewPortForUprightTexture() {
        if let pb = uprightPixelBuffer {
            setViewPortForTexture(pb)
        }
    }
    
    var uprightWidth : Int {
        if let pb = uprightPixelBuffer {
            return CVPixelBufferGetWidth(pb)
        } else {
            return 1
        }
    }
    
    var uprightHeight : Int {
        if let pb = uprightPixelBuffer {
            return CVPixelBufferGetHeight(pb)
        } else {
            return 1
        }
    }
    
    func uprightPixelBufferAsCamImg() -> (CamImage?, ()->()) {
        // Returns the CamImage AND a closure to tidy up (re-lock the PixelBuffer)
        if let pb = uprightPixelBuffer {
            CVPixelBufferLockBaseAddress(pb, kCVPixelBufferLock_ReadOnly)
            let ptr = UnsafeMutablePointer<UInt8>(CVPixelBufferGetBaseAddress(pb))
            let width = CVPixelBufferGetWidth(pb)
            let height = CVPixelBufferGetHeight(pb)
            let rsize = CVPixelBufferGetBytesPerRow(pb)
            return (
                CamImage(pixels: ptr, width: Int32(width), height: Int32(height), channels: 4, rowSize: Int32(rsize)),
                { () -> () in
                    if let pb = self.uprightPixelBuffer {
                        CVPixelBufferUnlockBaseAddress(pb, kCVPixelBufferLock_ReadOnly)
                    }
                })
            
        } else {
            return (nil, { () -> () in })
        }
    }
    var uprightRect : CGRect? {
        if let pb = uprightPixelBuffer {
            let width = CVPixelBufferGetWidth(pb)
            let height = CVPixelBufferGetHeight(pb)
            return CGRectMake(0, 0, CGFloat(width), CGFloat(height))
        } else {
            return nil
        }
    }
    
    // MARK: generate buffer for smaller textures
    func makeSmallerPixelBuffer(withWidth inWidth : Int, andHeight inHeight : Int, andScale scale : Int = 2) {
        smallPixelBuffer = nil
        smallTexture = nil
        CVOpenGLESTextureCacheFlush(textureCache!, 0)
        
        let height = inHeight / scale
        let width = inWidth / scale
        
        CVOpenGLESTextureCacheFlush(textureCache!, 0)
        let size = CGSizeMake(CGFloat(width), CGFloat(height))
        
        glActiveTexture(GLenum(GL_TEXTURE3)) // upright texture lives in texture 2
        generatePixelBuffer(&smallPixelBuffer, andTexture: &smallTexture, withSize: size)
        glBindTexture(CVOpenGLESTextureGetTarget(smallTexture!), CVOpenGLESTextureGetName(smallTexture!))
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_S), GL_CLAMP_TO_EDGE)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_T), GL_CLAMP_TO_EDGE)
        glActiveTexture(GLenum(GL_TEXTURE0))
    }
    func bindSmallerTextureAsOutput() {
        if smallTexture != nil {
            glFramebufferTexture2D(GLenum(GL_FRAMEBUFFER), GLenum(GL_COLOR_ATTACHMENT0), GLenum(GL_TEXTURE_2D), CVOpenGLESTextureGetName(smallTexture!), 0);
        }
    }
    func bindSmallerTextureToSlot(textureSlot : GLint) {
        glUniform1i(textureSlot, 3)
    }
    func setViewPortForSmallerTexture() {
        if let pb = smallPixelBuffer {
            setViewPortForTexture(pb)
        }
    }
    func smallPixelBufferAsCamImg() -> (CamImage?, ()->()) {
        if let pb = smallPixelBuffer {
            CVPixelBufferLockBaseAddress(pb, kCVPixelBufferLock_ReadOnly)
            let ptr = UnsafeMutablePointer<UInt8>(CVPixelBufferGetBaseAddress(pb))
            let width = CVPixelBufferGetWidth(pb)
            let height = CVPixelBufferGetHeight(pb)
            let rsize = CVPixelBufferGetBytesPerRow(pb)
            return (
                CamImage(pixels: ptr, width: Int32(width), height: Int32(height), channels: 4, rowSize: Int32(rsize)),
                { () -> () in
                    if let pb = self.smallPixelBuffer {
                        CVPixelBufferUnlockBaseAddress(pb, kCVPixelBufferLock_ReadOnly)
                    }
            })
            
        } else {
            return (nil, { () -> () in })
        }
    }
    
    
    // MARK: setup for non-CVPixel backed textures
    func setupNormalTextures() {
        glGenTextures(1, &hblurTexture)
        glGenTextures(1, &vblurTexture)

    }
    
    // MARK: generate buffer for hblur - GL_TEXTURE4
    func makeHBlurTexture(withWidth inWidth : Int, andHeight inHeight : Int, andScale scale : Int = 2) {
        hblurwidth = inWidth / scale
        hblurheight = inHeight / scale
        glActiveTexture(GLenum(GL_TEXTURE4))
        glBindTexture(GLenum(GL_TEXTURE_2D), hblurTexture)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_S), GL_CLAMP_TO_EDGE)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_T), GL_CLAMP_TO_EDGE)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_MAG_FILTER), GL_LINEAR)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_MIN_FILTER), GL_LINEAR)
        glTexImage2D(
            GLenum(GL_TEXTURE_2D),
            0,
            GLint(GL_RGBA),
            GLsizei(hblurwidth),
            GLsizei(hblurheight),
            0,
            GLenum(GL_RGBA),
            GLenum(GL_UNSIGNED_BYTE),
            nil);
        glActiveTexture(GLenum(GL_TEXTURE0))
        glBindTexture(GLenum(GL_TEXTURE_2D), 0)
        
    }
    func bindHBlurTextureAsOutput() {
        glFramebufferTexture2D(GLenum(GL_FRAMEBUFFER), GLenum(GL_COLOR_ATTACHMENT0), GLenum(GL_TEXTURE_2D), hblurTexture, 0);
    }
    func bindHBlurTextureToSlot(textureSlot : GLint) {
        glUniform1i(textureSlot, 4)
    }
    func setViewPortForHBlurTexture() {
        glViewport(
            0,
            0,
            GLsizei(hblurwidth),
            GLsizei(hblurheight)
        )
    }
    
    // MARK: generate buffer for vblur - GL_TEXTURE5
    func makeVBlurTexture(withWidth inWidth : Int, andHeight inHeight : Int, andScale scale : Int = 2) {
        vblurwidth = inWidth / scale
        vblurheight = inHeight / scale
        glActiveTexture(GLenum(GL_TEXTURE0))
        glBindTexture(GLenum(GL_TEXTURE_2D), vblurTexture)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_S), GL_CLAMP_TO_EDGE)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_T), GL_CLAMP_TO_EDGE)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_MAG_FILTER), GL_LINEAR)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_MIN_FILTER), GL_LINEAR)
        glTexImage2D(
            GLenum(GL_TEXTURE_2D),
            0,
            GLint(GL_RGBA),
            GLsizei(vblurwidth),
            GLsizei(vblurheight),
            0,
            GLenum(GL_RGBA),
            GLenum(GL_UNSIGNED_BYTE),
            nil);
        glActiveTexture(GLenum(GL_TEXTURE0))
        glBindTexture(GLenum(GL_TEXTURE_2D), 0)
        
    }
    func bindVBlurTextureAsOutput() {
        glFramebufferTexture2D(GLenum(GL_FRAMEBUFFER), GLenum(GL_COLOR_ATTACHMENT0), GLenum(GL_TEXTURE_2D), vblurTexture, 0);
    }
    func bindVBlurTextureToSlot(textureSlot : GLint) {
        glActiveTexture(GLenum(GL_TEXTURE5))
        glBindTexture(GLenum(GL_TEXTURE_2D), vblurTexture)
        glUniform1i(textureSlot, 5)
    }
    func setViewPortForVBlurTexture() {
        glViewport(
            0,
            0,
            GLsizei(vblurwidth),
            GLsizei(vblurheight)
        )
    }
    
    // MARK: generate buffer for compositing to
    func makeOutputPixelBuffer(withWidth width : Int, andHeight height : Int) {
        outputTexture = nil
        outputPixelBuffer = nil
        
        CVOpenGLESTextureCacheFlush(textureCache!, 0)
        let size = CGSizeMake(CGFloat(width), CGFloat(height))
        glActiveTexture(GLenum(GL_TEXTURE6)) // outputTexture lives in 6
        generatePixelBuffer(&outputPixelBuffer, andTexture: &outputTexture, withSize: size)
        glBindTexture(CVOpenGLESTextureGetTarget(outputTexture!), CVOpenGLESTextureGetName(outputTexture!))
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_S), GL_CLAMP_TO_EDGE)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_T), GL_CLAMP_TO_EDGE)
        glActiveTexture(GLenum(GL_TEXTURE0))
    }
    func bindOutputTextureAsOutput() {
        if outputTexture != nil {
            glFramebufferTexture2D(GLenum(GL_FRAMEBUFFER), GLenum(GL_COLOR_ATTACHMENT0), GLenum(GL_TEXTURE_2D), CVOpenGLESTextureGetName(outputTexture!), 0);
        }
    }
    func bindOutputTextureToSlot(textureSlot : GLint) {
        glUniform1i(textureSlot, 6)
    }
    func setViewPortForOutputTexture() {
        if let pb = outputPixelBuffer {
            setViewPortForTexture(pb)
        }
    }
    
    // MARK: Setupviewport to fill a given texture
    func setViewPortForTexture(pb : CVPixelBufferRef) {
        glViewport(
            0,
            0,
            GLsizei(CVPixelBufferGetWidth(pb)),
            GLsizei(CVPixelBufferGetHeight(pb))
        )
    }
    
    // MARK: Save a given pixel buffer to camera roll
    func savePixelBuffer(pb : CVPixelBufferRef) {
        //print("savePixelBuffer")
        CVPixelBufferLockBaseAddress(pb, 0)
        let addr = UnsafeMutablePointer<UInt8>(CVPixelBufferGetBaseAddress(pb))
        let rw = CVPixelBufferGetBytesPerRow(pb)
        let r = CVPixelBufferGetHeight(pb)
        for idx in 0..<((rw * r)/4) {
            addr[idx * 4 + 3] = 255
        }
        let ciImage = CIImage(CVPixelBuffer: pb)
        let tmpContext = CIContext()
        let width = CGFloat(CVPixelBufferGetWidth(pb))
        let height = CGFloat(CVPixelBufferGetHeight(pb))
        let videoImage = tmpContext.createCGImage(ciImage, fromRect: CGRectMake(0, 0, width, height))
        let uiImage = UIImage(CGImage: videoImage)
        UIImageWriteToSavedPhotosAlbum(uiImage, nil, nil, nil)
        CVPixelBufferUnlockBaseAddress(pb, 0)
    }
    
    
    func generatePixelBuffer(inout buffer : CVPixelBufferRef?, inout andTexture texture : CVOpenGLESTextureRef?, withSize size : CGSize) {
        let options = [
            kCVPixelBufferCGImageCompatibilityKey as String: false,
            kCVPixelBufferCGBitmapContextCompatibilityKey as String: false,
            kCVPixelFormatOpenGLESCompatibility as String: true,
            kCVPixelBufferIOSurfacePropertiesKey as String: [NSObject: NSObject]()
        ]
        
        let height = size.height
        let width = size.width
        
        let status = CVPixelBufferCreate(kCFAllocatorDefault, Int(width), Int(height), kCVPixelFormatType_32BGRA, options, &buffer)
        if status != kCVReturnSuccess {
            //print("Pixel buffer with image failed creating CVPixelBuffer with error \(status)")
//            exit(1)
        }
//        guard let _ = buffer else {
//            print("Pixel buffer did not allocate")
//            exit(1)
//        }
        _ = CVOpenGLESTextureCacheCreateTextureFromImage(
            kCFAllocatorDefault,
            textureCache!,
            buffer!,
            nil,
            GLenum(GL_TEXTURE_2D),
            GLint(GL_RGBA),
            GLsizei(width),
            GLsizei(height),
            GLenum(GL_BGRA),
            GLenum(GL_UNSIGNED_BYTE),
            0,
            &texture)
//        guard res == kCVReturnSuccess else {
//            print("Create texture from image failed with code \(res)")
//            exit(1)
//        }
    }
    
//    // MARK: Functions to load robot eye texture
    func loadEyeTexture() {
        let opt:[String : NSNumber] = [GLKTextureLoaderApplyPremultiplication : true, GLKTextureLoaderGenerateMipmaps: false]
        let path = NSBundle.mainBundle().pathForResource("overlay", ofType: "png")
        do {
            glGetError()
            glActiveTexture(GLenum(GL_TEXTURE7))
            eyeTexture = try GLKTextureLoader.textureWithContentsOfFile(path!, options: opt)
            glBindTexture(eyeTexture!.name, eyeTexture!.target)
            glActiveTexture(0)
        } catch {
            print("failed to load robot eye")
        }
    }
    
    func loadFacialTattooTexture() {
        glGetError()
        let opt:[String : NSNumber] = [GLKTextureLoaderApplyPremultiplication : true, GLKTextureLoaderGenerateMipmaps: false]
        let path = NSBundle.mainBundle().pathForResource("face_tattoo", ofType: "png")
        do {
            glGetError()
            //glActiveTexture(GLenum(GL_TEXTURE9))
            facialTattooTexture = try GLKTextureLoader.textureWithContentsOfFile(path!, options: opt)
            if facialTattooTexture  == nil {
                print("face_tattoo texture is nil")
            }
            //glBindTexture(rednoseTexture!.name, rednoseTexture!.target)
            //glActiveTexture(0)
            
            if facialTattooTexture != nil {
                print("there is no texture for face tattoo")
            } else {
                //print("there is texture for face tattoo")
            }
            
        } catch let error {
            print(error)
            print("failed to load face tattoo")
        }
    }
    
    func loadGasmaskTexture() {
        glGetError()
        let opt:[String : NSNumber] = [GLKTextureLoaderApplyPremultiplication : true, GLKTextureLoaderGenerateMipmaps: false]
        let path = NSBundle.mainBundle().pathForResource("gasmask", ofType: "png")
        do {
            glGetError()
            //glActiveTexture(GLenum(GL_TEXTURE9))
            
            gasMaskTexture = try GLKTextureLoader.textureWithContentsOfFile(path!, options: opt)
            if gasMaskTexture  == nil {
                print("gas mask texture is nil")
            }
            
            if gasMaskTexture != nil {
                print("there is no texture for mask texture")
            } else {
                //print("there is texture for face tattoo")
            }
            
        } catch let error {
            print(error)
            print("failed to load face tattoo")
        }
    }
    
    
    
    // MARK: Functions to load robot eye texture
    func loadRedNoseTexture() {
        glGetError()
        let opt:[String : NSNumber] = [GLKTextureLoaderApplyPremultiplication : true, GLKTextureLoaderGenerateMipmaps: false]
        let path = NSBundle.mainBundle().pathForResource("rednose", ofType: "png")
        do {
            glGetError()
            //glActiveTexture(GLenum(GL_TEXTURE9))
            rednoseTexture = try GLKTextureLoader.textureWithContentsOfFile(path!, options: opt)
            if rednoseTexture == nil {
                print("rednose texture is nil")
            }
            //glBindTexture(rednoseTexture!.name, rednoseTexture!.target)
            //glActiveTexture(0)
            
            if moTexture != nil {
                print("there is no texture for rednose")
            } else {
                //print("there is texture for rednose")
            }
            
        } catch let error {
            print(error)
            print("failed to load rednose")
        }
    }
    
    func bindRoboEyeToSlot(textureSlot : GLint) {
        glActiveTexture(GLenum(GL_TEXTURE7))
        glBindTexture(eyeTexture!.name, eyeTexture!.target)
        glUniform1i(textureSlot, 7)
    }
    
    // MARK: Functions to load robot face texture
    func loadRoboFaceTexture() {
        //print("load robot texture")
        let opt:[String : NSNumber] = [GLKTextureLoaderApplyPremultiplication : true, GLKTextureLoaderGenerateMipmaps: false]
        let path = NSBundle.mainBundle().pathForResource("overlay3", ofType: "png")

        do {
            
            print(glGetError())
            faceTexture = try GLKTextureLoader.textureWithContentsOfFile(path!, options: opt)
            glActiveTexture(GLenum(GL_TEXTURE8))
            glBindTexture(GLenum(GL_TEXTURE_2D), faceTexture!.name)
            glActiveTexture(0)
            

           
            //glActiveTexture(0)
        } catch let error {
            print(error)
            print("failed to load robot mask")
        
        }
    }
    
    func bindRoboFaceToSlot(textureSlot : GLint) {
        glActiveTexture(GLenum(GL_TEXTURE7)) // teeth texture lives in texture 7
        glBindTexture(GLenum(GL_TEXTURE_2D), self.eyeTexture!.name)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_S), GL_CLAMP_TO_EDGE)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_T), GL_CLAMP_TO_EDGE)
        glActiveTexture(GLenum(GL_TEXTURE0))
        glUniform1i(textureSlot, 7)
        glFramebufferTexture2D(GLenum(GL_FRAMEBUFFER), GLenum(GL_COLOR_ATTACHMENT0), GLenum(GL_TEXTURE_2D), eyeTexture!.name, 0);

    }
    
    // MARK MOUSTACHE
    func loadMoFaceTexture() {
        glGetError()
        //print("loadMoFaceTexture")
        let opt:[String : NSNumber] = [GLKTextureLoaderApplyPremultiplication : true, GLKTextureLoaderGenerateMipmaps: false]
        let path = NSBundle.mainBundle().pathForResource("moustache2", ofType: "png")

        do {
            
            print(glGetError())
            self.moTexture = try GLKTextureLoader.textureWithContentsOfFile(path!, options: opt)
            glActiveTexture(GLenum(GL_TEXTURE8))
            glBindTexture(GLenum(GL_TEXTURE_2D), self.moTexture!.name)
            glActiveTexture(0)

            /*
            if moTexture != nil {
                print("there is a texture for mo face")
            } else {
                print("motexture exists")
            }
            */

        } catch let error {
            //tex = nil
            print("error loading mo face")
            print(error)
        }
    }

    func loadMaskFaceTexture() {
        glGetError()
        //print("loadMaskFaceTexture")
        let opt:[String : NSNumber] = [GLKTextureLoaderApplyPremultiplication : true, GLKTextureLoaderGenerateMipmaps: false]
        let path = NSBundle.mainBundle().pathForResource("mask", ofType: "png")
        
        do {
            
            print(glGetError())
            self.maskTexture = try GLKTextureLoader.textureWithContentsOfFile(path!, options: opt)
            glActiveTexture(GLenum(GL_TEXTURE8))
            glBindTexture(GLenum(GL_TEXTURE_2D), self.maskTexture!.name)
            glActiveTexture(0)
        
            
        } catch let error {
            //tex = nil
            print("error loading maskfacee")
            print(error)
        }
    }
    
    
    func bindMaskFaceToSlot(textureSlot : GLint) {
        //print("bindMaskFaceToSlot")
        glActiveTexture(GLenum(GL_TEXTURE7)) // teeth texture lives in texture 7
        glBindTexture(GLenum(GL_TEXTURE_2D),  maskTexture!.name)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_S), GL_CLAMP_TO_EDGE)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_T), GL_CLAMP_TO_EDGE)
        glActiveTexture(GLenum(GL_TEXTURE0))
        glUniform1i(textureSlot, 7)
        glFramebufferTexture2D(GLenum(GL_FRAMEBUFFER), GLenum(GL_COLOR_ATTACHMENT0), GLenum(GL_TEXTURE_2D), maskTexture!.name, 0);
        
    } 
    
    func bindRedNoseToSlot(textureSlot : GLint) {
        //print("bindRedNoseToSlot")
        if rednoseTexture == nil {
            print("rednose texture is nil")
        }
        
        glActiveTexture(GLenum(GL_TEXTURE7)) // teeth texture lives in texture 7
        glBindTexture(GLenum(GL_TEXTURE_2D), rednoseTexture!.name)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_S), GL_CLAMP_TO_EDGE)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_T), GL_CLAMP_TO_EDGE)
        glActiveTexture(GLenum(GL_TEXTURE0))
        glUniform1i(textureSlot, 7)
        glFramebufferTexture2D(GLenum(GL_FRAMEBUFFER), GLenum(GL_COLOR_ATTACHMENT0), GLenum(GL_TEXTURE_2D), rednoseTexture!.name, 0);
        
    }
    
    func bindFacialTattooToSlot(textureSlot : GLint) {
        //print("bindRedNoseToSlot")
        if facialTattooTexture == nil {
            print("rednose texture is nil")
        }
        
        glActiveTexture(GLenum(GL_TEXTURE7)) // teeth texture lives in texture 7
        glBindTexture(GLenum(GL_TEXTURE_2D), facialTattooTexture!.name)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_S), GL_CLAMP_TO_EDGE)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_T), GL_CLAMP_TO_EDGE)
        glActiveTexture(GLenum(GL_TEXTURE0))
        glUniform1i(textureSlot, 7)
        glFramebufferTexture2D(GLenum(GL_FRAMEBUFFER), GLenum(GL_COLOR_ATTACHMENT0), GLenum(GL_TEXTURE_2D), facialTattooTexture!.name, 0);
    }
    
    func bindGasmaskToSlot(textureSlot : GLint) {
        //print("bindRedNoseToSlot")
        if gasMaskTexture == nil {
            print("gas mask texture is nil")
        }
        
        glActiveTexture(GLenum(GL_TEXTURE7)) // teeth texture lives in texture 7
        glBindTexture(GLenum(GL_TEXTURE_2D), gasMaskTexture!.name)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_S), GL_CLAMP_TO_EDGE)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_T), GL_CLAMP_TO_EDGE)
        glActiveTexture(GLenum(GL_TEXTURE0))
        glUniform1i(textureSlot, 7)
        glFramebufferTexture2D(GLenum(GL_FRAMEBUFFER), GLenum(GL_COLOR_ATTACHMENT0), GLenum(GL_TEXTURE_2D), gasMaskTexture!.name, 0);
    }

    func bindMoFaceToSlot(textureSlot : GLint) {
        //print("bindMoFaceToSlot")
        glActiveTexture(GLenum(GL_TEXTURE7)) // teeth texture lives in texture 7
        glBindTexture(GLenum(GL_TEXTURE_2D), self.moTexture!.name)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_S), GL_CLAMP_TO_EDGE)
        glTexParameteri(GLenum(GL_TEXTURE_2D), GLenum(GL_TEXTURE_WRAP_T), GL_CLAMP_TO_EDGE)
        glActiveTexture(GLenum(GL_TEXTURE0))
        glUniform1i(textureSlot, 7)
        glFramebufferTexture2D(GLenum(GL_FRAMEBUFFER), GLenum(GL_COLOR_ATTACHMENT0), GLenum(GL_TEXTURE_2D), moTexture!.name, 0);
        
    }
    
}