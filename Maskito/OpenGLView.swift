//
//  OpenGLView.swift
//

import Foundation
import UIKit
import QuartzCore
import OpenGLES
import GLKit
import CoreVideo
import CoreGraphics
import CoreFoundation
import AVFoundation
import Photos

import ImageIO

import VideoToolbox

typealias ImagePosition = (GLfloat, GLfloat, GLfloat)
typealias TexturePosition = (GLfloat, GLfloat)
//typealias Color = (CFloat, CFloat)
//var a : CGFloat = 0
//var b : CFloat = a


struct Coordinate {
    var xyz : ImagePosition
    var uv : TexturePosition
    var alpha : GLfloat
}


var leye_dlib = [36, 37, 38, 39, 40, 41];
var reye_dlib = [42, 43, 44, 45, 46, 47];

var outlinePoints : [Float] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26];

var FaceIndicesSmall : [GLubyte] = [
    1, 2, 0,
    0, 2, 3,
]


func time<O>(name: String, f : ()->O )-> O {
    let startTime = NSDate()
    let rez = f()
    let deltaT = NSDate().timeIntervalSinceDate(startTime)
    print("\(name) took \(deltaT)")
    return rez
}




class OpenGLView: UIView {
    @IBOutlet weak var uiContainer: UIView!

    var eaglLayer: CAEAGLLayer!
    var context: EAGLContext!
    
    var captureManager : CaptureManager?
    var renderer : Renderer?
    
    var camera : Int = 1
    let CAMERA_KEY = "PixurgeryCamera"
    
    /* Class Methods
    ------------------------------------------*/
    
    override class func layerClass() -> AnyClass {
        // In order for our view to display OpenGL content, we need to set it's
        //   default layer to be a CAEAGLayer
        return CAEAGLLayer.self
    }
    
    
    /* Lifecycle
    ------------------------------------------*/
    
    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
        UIApplication.sharedApplication().idleTimerDisabled = true
        
        let delegate = UIApplication.sharedApplication().delegate as! AppDelegate
        delegate.view = self
        
        self.setupLayer()
        self.setupContext()
        
        if NSUserDefaults().objectForKey(CAMERA_KEY) != nil {
            camera = NSUserDefaults().integerForKey(CAMERA_KEY)
        }
        
        self.renderer = Renderer(withContext: context, andLayer: eaglLayer, andCamera: camera)
        
        setupPipelineWithCamera(camera, andRenderer: renderer!)
        

        /*
        let time = dispatch_time(dispatch_time_t(DISPATCH_TIME_NOW), 4 * Int64(NSEC_PER_SEC))
        dispatch_after(time, dispatch_get_main_queue()) {
            //put your code which should be executed with a delay here
            
            let rtmpConnection:RTMPConnection = RTMPConnection()
            let rtmpStream = RTMPStream(rtmpConnection: rtmpConnection)
            rtmpStream.syncOrientation = true
            
            rtmpStream.videoSettings = [
                "width": 365, // video output width
                "height": 667, // video output height
                "profileLevel": kVTCompressionPropertyKey_RealTime,
            ]
            
            rtmpStream.audioSettings = [
                "bitrate": 128000
                
            ]
            
            rtmpStream.attachAudio(AVCaptureDevice.defaultDeviceWithMediaType(AVMediaTypeAudio))
            
            let sc = ScreenCaptureSession()

            sc.view = self
            //subviews[self.subviews!count-1]
            //        sc.startRunning()
            
            rtmpStream.attachScreen(sc)
            
            rtmpConnection.connect("rtmp://54.183.189.243/live")
            rtmpStream.publish("1")
            
            
            
        }
        */
        
        


        
        
    }
    
    
    /* Gesture recogniser
    ------------------------------------------*/
    
    func singleTap(rec : UITapGestureRecognizer) {
        self.renderer!.doFaceBlur = !(self.renderer!.doFaceBlur)
        self.renderer!.scheduleSave()
    }
    
    /* Instance Methods
    ------------------------------------------*/
    
    func setupPipelineWithCamera(camera : Int, andRenderer renderer : Renderer) {
        if CaptureManager.devices().count > 0 { // check if we're running in the sim to debug ui shit
            let device = CaptureManager.devices()[camera]
            for format in device.formats as! [AVCaptureDeviceFormat] {
                if CMVideoFormatDescriptionGetDimensions(format.formatDescription).height == 960 {
                    do {
                        try device.lockForConfiguration()
                        device.activeFormat = format
                        device.unlockForConfiguration()
                    } catch {
                        print("Could not set config")
                    }
                }
            }
            
            self.captureManager = CaptureManager(withDevice: device)
            renderer.camera = camera
            
            do {
                try self.captureManager?.connectToRenderer(renderer)
            } catch {
                print("Capture manager could not connect to renderer")
//                exit(1)
            }
            
            self.captureManager?.start()
        }
    }
    
    func toggleCamera() {
        self.captureManager!.stop()
        self.captureManager = nil
        
        if self.camera == 0{
            camera = 1
        } else {
            camera = 0
        }
        NSUserDefaults().setInteger(camera, forKey: CAMERA_KEY)
        self.setupPipelineWithCamera(camera, andRenderer: renderer!)
    }
    
    func toggleFlash() {
        let device = AVCaptureDevice.defaultDeviceWithMediaType(AVMediaTypeVideo)
        if (device.hasTorch) {
            do {
                try device.lockForConfiguration()
                if (device.torchMode == AVCaptureTorchMode.On) {
                    device.torchMode = AVCaptureTorchMode.Off
                } else {
                    do {
                        try device.setTorchModeOnWithLevel(1.0)
                    } catch {
                        print(error)
                    }
                }
                device.unlockForConfiguration()
            } catch {
                print(error)
            }
        }
        
    }
    
    @IBOutlet weak var overlayImage: UIImageView!
    @IBOutlet weak var instructions: UILabel!
    func hideInstructions(hidden : Bool) {
        instructions.hidden = hidden
        uiContainer.hidden = !hidden
        switch UIApplication.sharedApplication().statusBarOrientation {
        case .Portrait:
            overlayImage.hidden = hidden
            instructions.text = "Face the screen\nwith a neutral expression\nand hold for 3 seconds"
        case _:
            overlayImage.hidden = true
            instructions.text = "Rotate the device\nto portrait\nand try again"
        }
        
    }
    
    func setTextForCount(count : Int) {
        instructions.text = "Face the screen\nwith a neutral expression\nand hold for \(count) seconds"
    }
    
    func setupLayer() {
        // CALayer's are, by default, non-opaque, which is 'bad for performance with OpenGL',
        //   so let's set our CAEAGLLayer layer to be opaque.
        self.eaglLayer	= self.layer as! CAEAGLLayer
        self.eaglLayer.opaque = true
        self.contentScaleFactor = UIScreen.mainScreen().scale
        self.eaglLayer.contentsScale = UIScreen.mainScreen().scale

        //if #available(iOS 8.0, *) {
        //    self.eaglLayer.bounds.size.width = UIScreen.mainScreen().fixedCoordinateSpace.bounds.width
        //    self.eaglLayer.bounds.size.height = UIScreen.mainScreen().fixedCoordinateSpace.bounds.height
        //} else {
            self.eaglLayer.bounds.size.width = UIScreen.mainScreen().bounds.width
            self.eaglLayer.bounds.size.height = UIScreen.mainScreen().bounds.height
        //}
        
    }
    
    func setupContext() {
        // Just like with CoreGraphics, in order to do much with OpenGL, we need a context.
        //   Here we create a new context with the version of the rendering API we want and
        //   tells OpenGL that when we draw, we want to do so within this context.
        let api: EAGLRenderingAPI = EAGLRenderingAPI.OpenGLES2
        self.context = EAGLContext(API: api)
        
        if (self.context == nil) {
            print("Failed to initialize OpenGLES 3.0 context!")
//            exit(1)
        }
        
        if (!EAGLContext.setCurrentContext(self.context)) {
            print("Failed to set current OpenGL context!")
//            exit(1)
        }
    }
}
///////////////////////////////////////

