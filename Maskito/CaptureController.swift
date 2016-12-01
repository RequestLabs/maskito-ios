//
//  CaptureController.swift
//

import Foundation
import StoreKit
import Social
import AssetsLibrary

import AVFoundation
import VideoToolbox

class CaptureController : UIViewController, UICollectionViewDataSource, UICollectionViewDelegate {
    let albumName = "Pixurgery"
    let VIDEO_MODE = "PixurgeryVideoMode"
    var delegate : AppDelegate? = nil
    var allowCapture : Bool = false
    let titles = ["VIDEO"]
    
    var uithumb : UIImage? = nil
    
    var openglview: OpenGLView?
    
    @IBOutlet weak var GLView: OpenGLView!
    @IBOutlet weak var capture: UIButton!
    @IBOutlet weak var selectCamera: UIButton!
    @IBOutlet weak var selectFlash: UIButton!
    @IBOutlet weak var filterPicker: CategoryRow!
 
    
    @IBAction func flashPressed(sender: AnyObject) {
        //delegate?.toggleFlash()
        Nav.handle()
    }
    
    @IBAction func capturePressed(sender: AnyObject) {
        print("capture pressed")
        //openglview?.renderer?.textureManager?.saveOutput()
        self.capture.imageView!.image = capture.imageView?.image?.imageWithRenderingMode(UIImageRenderingMode.AlwaysTemplate)
        if delegate!.syncro.capturing {
            print("capturing white")
            delegate!.syncro.capturing = false
            self.capture.tintColor = UIColor.whiteColor()
            self.performSegueWithIdentifier("videoViewer", sender: self)
            self.capture.layer.removeAllAnimations()
        } else {
            delegate!.syncro.capturing = true
            //sender.imageView!?.image
            //self.capture.imageView!.image = UIImage(named: "pinchLogo")
            //sender.setImage(nil, forState: UIControlState.Normal)
            sender.setImage(UIImage(named: "recordBtn"), forState: UIControlState.Normal)

            // Track Capture
            Analytics.push("recordBtn")

            
        }
    }
    
    
    @IBAction func selectCameraPressed(sender: AnyObject) {
        
        delegate?.toggleCamera()
        Analytics.push("toggleCamera")
    }

    func updateUI() {
        
    }
    
    // MARK COLLECTION PICKER
    func collectionView(collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        return 12
    }
    
    func collectionView(collectionView: UICollectionView, cellForItemAtIndexPath indexPath: NSIndexPath) -> UICollectionViewCell {
        let cell = collectionView.dequeueReusableCellWithReuseIdentifier("filterCell", forIndexPath: indexPath) as! FilterCell
        
        if indexPath.row == 0 {
            cell.imageView.image = UIImage(named: "animeLogo")
        }
        if indexPath.row == 1 {
            cell.imageView.image = UIImage(named: "faceswapLogo")
        }
        if indexPath.row == 2 {
            cell.imageView.image = UIImage(named: "pinchLogo")
        }
        if indexPath.row == 3 {
            cell.imageView.image = UIImage(named: "wobbleLogo")
        }
        if indexPath.row == 4 {
            cell.imageView.image = UIImage(named: "sparkleLogo")
        }
        if indexPath.row == 5 {
            cell.imageView.image = UIImage(named: "biglipsLogo")
        }
        if indexPath.row == 6 {
            cell.imageView.image = UIImage(named: "bignoseLogo")
        }
        if indexPath.row == 7 {
            cell.imageView.image = UIImage(named: "rednoseLogo")
        }
        if indexPath.row == 8 {
            cell.imageView.image = UIImage(named: "moustacheLogo")
        }
        if indexPath.row == 9 {
            cell.imageView.image = UIImage(named: "maskLogo")
        }
        if indexPath.row == 10 {
            cell.imageView.image = UIImage(named: "facialTattooLogo")
        }
        if indexPath.row == 11 {
            cell.imageView.image = UIImage(named: "gasmaskLogo")
        }
        return cell
    }
    
    
    func collectionView(collectionView: UICollectionView, didSelectItemAtIndexPath indexPath: NSIndexPath) {
        if indexPath.row == 0 {
            print("show silly")
            openglview?.renderer?.destroyTexture()
            delegate!.syncro.warp = WarpType.SILLY
            Analytics.push("Warp.SILLY")
        }
        if indexPath.row == 1 {
            print("show swap")
            openglview?.renderer?.destroyTexture()
            delegate!.syncro.warp = WarpType.SWAP
            Analytics.push("Warp.SWAP")
        }
        if indexPath.row == 2 {
            print("show tiny")
            openglview?.renderer?.destroyTexture()
            delegate!.syncro.warp = WarpType.TINY
            Analytics.push("Warp.TINY")
        }
        if indexPath.row == 3 {
            print("show dynamic")
            openglview?.renderer?.destroyTexture()
            delegate!.syncro.warp = WarpType.DYNAMIC
            Analytics.push("Warp.DYNAMIC")
        }
        if indexPath.row == 4 {
            print("show handsome")
            openglview?.renderer?.destroyTexture()
            delegate!.syncro.warp = WarpType.PRETTY
            Analytics.push("Warp.PRETTY")
        }
        if indexPath.row == 5 {
            openglview?.renderer?.destroyTexture()
            delegate!.syncro.warp = WarpType.BIGLIPS
            Analytics.push("Warp.BIGLIPS")
        }
        if indexPath.row == 6 {
            openglview?.renderer?.destroyTexture()
            delegate!.syncro.warp = WarpType.NOSE
            Analytics.push("Warp.NOSE")
        }
        if indexPath.row == 7 {
            openglview?.renderer?.destroyTexture()
            delegate!.syncro.warp = WarpType.REDNOSE
            Analytics.push("Warp.REDNOSE")
        }
        if indexPath.row == 8 {
            openglview?.renderer?.destroyTexture()
            delegate!.syncro.warp = WarpType.MOUSTACHE
            Analytics.push("Warp.MOUSTACHE")
        }
        if indexPath.row == 9 {
            openglview?.renderer?.destroyTexture()
            delegate!.syncro.warp = WarpType.MASK
            Analytics.push("Warp.MASK")
        }
        if indexPath.row == 10 {
            openglview?.renderer?.destroyTexture()
            delegate!.syncro.warp = WarpType.FACIALTATTOO
            Analytics.push("Warp.FACIALTATTOO")
        }
        if indexPath.row == 11 {
            openglview?.renderer?.destroyTexture()
            delegate!.syncro.warp = WarpType.GASMASK
            Analytics.push("Warp.GASMASK")
        }
        
    }
    
    func collectionView(collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, sizeForItemAtIndexPath indexPath: NSIndexPath) -> CGSize {
        let itemWidth = 80
        let itemHeight = 80
        return CGSize(width: itemWidth, height: itemHeight)
    }
    
    func setCam(item : Int) {
        switch item {
        case 0:
            delegate?.syncro.capture_type = .IMAGE
        case 1:
            delegate?.syncro.capture_type = .VIDEO
        case _:
            print("Shouldn't be here!")
        }
    }
    
    
    func didRotate() {
        switch UIDevice.currentDevice().orientation {
        case .LandscapeLeft:
            print("Landscape left")
            UIView.animateWithDuration(0.3, delay: 0.0,
                options: UIViewAnimationOptions.CurveEaseInOut,
                animations: {
                //self.capture.transform = CGAffineTransformMakeRotation(CGFloat(M_PI_2))
                self.selectCamera.transform = CGAffineTransformMakeRotation(CGFloat(M_PI_2))
                }, completion: nil)

        case .LandscapeRight:
            print("Landscape right")
            UIView.animateWithDuration(0.3, delay: 0.0,
                options: UIViewAnimationOptions.CurveEaseInOut,
                animations: {
                //self.capture.transform = CGAffineTransformMakeRotation(-CGFloat(M_PI_2))
                self.selectCamera.transform = CGAffineTransformMakeRotation(-CGFloat(M_PI_2))
            }, completion: nil)
                case .PortraitUpsideDown:
            print("Portrait Upside Down")
            UIView.animateWithDuration(0.3, delay: 0.0,
                options: UIViewAnimationOptions.CurveEaseInOut,
                animations: {
                //self.capture.transform = CGAffineTransformMakeRotation(CGFloat(M_PI))
                self.selectCamera.transform = CGAffineTransformMakeRotation(CGFloat(M_PI))
            }, completion: nil)
        case _:
            print("Portrait or other")
            UIView.animateWithDuration(0.3, delay: 0.0,
                options: UIViewAnimationOptions.CurveEaseInOut,
                animations: {
                //self.capture.transform = CGAffineTransformMakeRotation(CGFloat(0))
                self.selectCamera.transform = CGAffineTransformMakeRotation(CGFloat(0))
            }, completion: nil)

        }
    }
    
    
    
    override func viewWillAppear(animated: Bool) {
        super.viewWillAppear(true)
        //let tb = self.tabBarController as! TabBarController
        //tb.hidden()
        
        

        
        
        delegate = UIApplication.sharedApplication().delegate as? AppDelegate
        delegate!.ui = self

        // Add Tracking
        Analytics.view("CaptureViewController")

        


    }
    
    override func viewDidAppear(animated: Bool) {
        super.viewDidAppear(true)
        
        //openglview?.renderer?.destroyTexture()
        //delegate!.syncro.warp = WarpType.NONE
        
        let frame = UIScreen.mainScreen().bounds
        self.view.frame = frame
        
        

    }
    
    override func viewDidDisappear(animated: Bool) {
        super.viewDidDisappear(true)
        
    }
    override func viewDidLoad() {
        super.viewDidLoad()
        
        
        self.capture.transform = CGAffineTransformMakeRotation(CGFloat(0))
        self.selectCamera.transform = CGAffineTransformMakeRotation(CGFloat(0))
        
        self.setCam(1)
        self.GLView = self.openglview
        
        delegate = UIApplication.sharedApplication().delegate as? AppDelegate
        delegate!.ui = self
        
        self.filterPicker.delegate = self
        self.filterPicker.showsHorizontalScrollIndicator = false
        
        selectCamera.imageView?.contentMode = UIViewContentMode.ScaleAspectFit
        selectFlash.imageView?.contentMode = UIViewContentMode.ScaleAspectFit
        
    
    }
    
    
    func ownsWarp(owns : Bool) {
        if owns {
            self.capture.setTitle("", forState: UIControlState.Normal)
            allowCapture = true
        } else {
            delegate!.syncro.capturing = false
            self.capture.setTitle("🔒", forState: UIControlState.Normal)
            allowCapture = false
        }
    }
    
    func listSubviewsOfView(view:UIView) -> [UIView]{
        
        var views: [UIView] = []
        // Get the subviews of the view
        let subviews = view.subviews
        
        // Return if there are no subviews
        if subviews.count == 0 {
            return [UIView()]
        }
        
        for subview : AnyObject in subviews{
            
            views.append(subview as! UIView)
            // Do what you want to do with the subview

            // List the subviews of subview
            listSubviewsOfView(subview as! UIView)
        }
        return views
    }
    
    func enableUI() {
        self.selectCamera.hidden = false
    }
    
    func disableUI() {
        self.selectCamera.hidden = true
    }
    
    func recordTime(time : Int) {
        if time > 0 {
            //capture.setTitle(String(time), forState: .Normal)
        } else {
            //capture.setTitle("", forState: UIControlState.Normal)
        }
    }
    
    
    func scrollViewDidScroll(scrollView: UIScrollView) {
        // println("\(scrollView.contentOffset.x)")
    }
    
    override func shouldAutorotate() -> Bool {
        let del = UIApplication.sharedApplication().delegate as! AppDelegate
        if (del.syncro.capturing && del.syncro.capture_type == .VIDEO) {
            return false
        } else {
            return true
        }
    }
    
    // SEGUE
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
        if segue.identifier == "videoViewer" {
            if let dVC = segue.destinationViewController as? ViewerController {
                dVC.videoURL = self.delegate?.syncro.videoURL
            }
            //
        }
    }
    
}
