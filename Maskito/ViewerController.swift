//

import UIKit
import AVFoundation
import AVKit
import Social
import AssetsLibrary

class ViewerController: UIViewController, UICollectionViewDataSource, UICollectionViewDelegate {
    
    var d : AppDelegate? = nil
    
    @IBOutlet weak var shareCollection: CategoryRow!
    @IBOutlet weak var playerContainer: UIView!
  
    var videoURL: String!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        print("viewDidLoad")
        self.shareCollection.delegate = self
        self.shareCollection.showsHorizontalScrollIndicator = false

        
        d = UIApplication.sharedApplication().delegate as? AppDelegate
        
        let delayTime = dispatch_time(DISPATCH_TIME_NOW, Int64(1 * Double(NSEC_PER_SEC)))
        dispatch_after(delayTime, dispatch_get_main_queue()) {
            let url = NSURL(string: (self.d?.syncro.videoURL)!)
            let player = AVPlayer(URL: url!)
            let playerController = AVPlayerViewController()
            playerController.player = player
            playerController.showsPlaybackControls = false
            
            self.playerContainer.addSubview(playerController.view)
            playerController.view.frame = self.view.frame
            player.play()
        }
        
        // Tracking
        // Add Tracking
        Analytics.view("ViewerController")
    }
    
    // MARK COLLECTION PICKER
    func collectionView(collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        return 4
    }
    
    func collectionView(collectionView: UICollectionView, cellForItemAtIndexPath indexPath: NSIndexPath) -> UICollectionViewCell {
        let cell = collectionView.dequeueReusableCellWithReuseIdentifier("filterCell", forIndexPath: indexPath) as! FilterCell
        
        if indexPath.row == 0 {
            cell.imageView.image = UIImage(named: "backLogo")
        }
        if indexPath.row == 3 {
            cell.imageView.image = UIImage(named: "saveLogo")
        }
        if indexPath.row == 2 {
            cell.imageView.image = UIImage(named: "instagramLogo")
        }
        if indexPath.row == 1 {
            cell.imageView.image = UIImage(named: "imessageLogo")
        }
        return cell
    }
    
    func collectionView(collectionView: UICollectionView, didSelectItemAtIndexPath indexPath: NSIndexPath) {
        
        if indexPath.row == 0 {
            Analytics.push("BACK")
            //self.performSegueWithIdentifier("backFromShare", sender: self)
            User.completeOnboarding()
            Nav.handle()
        }
        
        if indexPath.row == 1 {
            Analytics.push("IMESSAGE")
            let videoLink = NSURL(fileURLWithPath: (self.d?.syncro.videoURL)!)
            let objectsToShare = [videoLink] //comment!, imageData!, myWebsite!]
            let activityVC = UIActivityViewController(activityItems: objectsToShare, applicationActivities: nil)
            activityVC.setValue("Video", forKey: "subject")
            self.presentViewController(activityVC, animated: true, completion: nil)
        }
        
        if indexPath.row == 3 {
            Analytics.push("SAVE")
            let videoLink = NSURL(fileURLWithPath: (self.d?.syncro.videoURL)!)
            let objectsToShare = [videoLink] //comment!, imageData!, myWebsite!]
            let activityVC = UIActivityViewController(activityItems: objectsToShare, applicationActivities: nil)
            activityVC.setValue("Video", forKey: "subject")
            self.presentViewController(activityVC, animated: true, completion: nil)
        }
        
        if indexPath.row == 2 {
            Analytics.push("INSTAGRAM")
            let captionString = "Uploaded via Mosquito App"
            let urlReference = self.d?.syncro.videoURL
            ALAssetsLibrary().writeVideoAtPathToSavedPhotosAlbum(NSURL(string: urlReference!), completionBlock: nil)
            let encodedURL = urlReference?.stringByAddingPercentEncodingWithAllowedCharacters(NSCharacterSet.alphanumericCharacterSet())
            let encodedCaption = captionString.stringByAddingPercentEncodingWithAllowedCharacters(NSCharacterSet.alphanumericCharacterSet())
            let urlString = String(format: "instagram://library?AssetPath=%@&InstagramCaption=%@", encodedURL!, encodedCaption!)
            let instagramURL = NSURL(string: urlString)
            
            if UIApplication.sharedApplication().canOpenURL(instagramURL!) {
                UIApplication.sharedApplication().openURL(instagramURL!)
            } else {
                print("Can't open Instagram")
            }
        }
        
    }
    
    func collectionView(collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, sizeForItemAtIndexPath indexPath: NSIndexPath) -> CGSize {
        let itemWidth = 80
        let itemHeight = 80
        return CGSize(width: itemWidth, height: itemHeight)
    }

    
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
}

