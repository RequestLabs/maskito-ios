//
//  PixelBufferTexture.swift
//

import Foundation
import CoreVideo
import CoreMedia

enum LockError: ErrorType {
    case LOCK
}

class PixelBufferTexture {
    var pixelBuffer : CVPixelBufferRef? = nil
    var textureRef : CVOpenGLESTextureRef? = nil
    
    init?(fromSampleBuffer sampleBuffer : CMSampleBuffer) {
        
    }
    
    init?(WithSize size : CGSize, andEAGLContext context : CVEAGLContext) {
        
        var tmpPixelBuffer : CVPixelBufferRef? = nil
        var tmpTextureRef : CVOpenGLESTextureRef? = nil
        
        let options = [
            kCVPixelBufferCGImageCompatibilityKey as String: false,
            kCVPixelBufferCGBitmapContextCompatibilityKey as String: false,
            kCVPixelFormatOpenGLESCompatibility as String: true,
            kCVPixelBufferIOSurfacePropertiesKey as String: [NSObject: NSObject]()
        ]
        
        let status = CVPixelBufferCreate(kCFAllocatorDefault, Int(size.height), Int(size.width), kCVPixelFormatType_32BGRA, options, &tmpPixelBuffer)
        if status != kCVReturnSuccess {
            print("Pixel buffer with image failed creating CVPixelBuffer with error \(status)")
        }
        if let pb = tmpPixelBuffer {
            pixelBuffer = pb
        } else {
            print("Pixel buffer did not allocate")
        }
        
        var texCacheRef : CVOpenGLESTextureCacheRef?
        let cacheStatus = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, nil, context, nil, &texCacheRef)
        if cacheStatus != kCVReturnSuccess {
            print("Creating texture cache failed with error \(cacheStatus)")
        }
        
        let res = CVOpenGLESTextureCacheCreateTextureFromImage(
            kCFAllocatorDefault,
            texCacheRef!,
            pixelBuffer!,
            nil,
            GLenum(GL_TEXTURE_2D),
            GLint(GL_RGBA),
            GLsizei(size.height),
            GLsizei(size.width),
            GLenum(GL_BGRA),
            GLenum(GL_UNSIGNED_BYTE),
            0,
            &tmpTextureRef)
        guard res == kCVReturnSuccess else {
            print("Create texture from image failed with code \(res)")
            return nil
        }
        if let tr = tmpTextureRef {
            textureRef = tr
        } else {
            print("TextureRef did not allocate")
            return nil
        }
    }
    
    var name : GLuint {
        return CVOpenGLESTextureGetName(textureRef!)
    }
    
    var target : GLuint {
        return CVOpenGLESTextureGetTarget(textureRef!)
    }
    
    func lockPixelBuffer() throws {
        if CVPixelBufferLockBaseAddress(pixelBuffer!, 0) != kCVReturnSuccess {
            throw LockError.LOCK
        }
    }
    
    var pixelBufferBaseAddress : UnsafeMutablePointer<Void>? {
        let addr = CVPixelBufferGetBaseAddress(pixelBuffer!)
        guard addr.hashValue != 0 else {
            return nil
        }
        return addr
    }
    
    func unlockPixelBuffer() throws {
        if CVPixelBufferUnlockBaseAddress(pixelBuffer!, 0) != kCVReturnSuccess {
            throw LockError.LOCK
        }
    }

    
}

func uiImageFromPixelBuffer(pixelBuffer: CVPixelBufferRef) -> UIImage {
    let ciImage = CIImage(CVPixelBuffer: pixelBuffer)
    let temporaryContext = CIContext()
    let cgImage = temporaryContext.createCGImage(ciImage, fromRect: CGRectMake(0, 0, CGFloat(CVPixelBufferGetWidth(pixelBuffer)), CGFloat(CVPixelBufferGetHeight(pixelBuffer))))
    let uiImage = UIImage(CGImage: cgImage)
    return uiImage
}
