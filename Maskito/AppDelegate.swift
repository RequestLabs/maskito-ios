//
//  AppDelegate.swift
//

import UIKit
import CoreData
import Contacts
import Alamofire

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate {

    func getDocumentsDirectory() -> NSString {
        let paths = NSSearchPathForDirectoriesInDomains(.DocumentDirectory, .UserDomainMask, true)
        let documentsDirectory = paths[0]
        return documentsDirectory
    }

    
    func setRecordTime(time : Int) {
        if let ui = ui {
            ui.recordTime(time)
        }
    }
    
    func redrawUI() {
        if let ui = ui {
            ui.updateUI()
        }
    }

    func disableUI() {
        if let ui = ui {
            ui.disableUI()
        }
    }
    func enableUI() {
        if let ui = ui {
            ui.enableUI()
        }
    }
    
    func toggleCamera() {
        if let view = view {
            view.toggleCamera()
        }
    }
    
    func toggleFlash() {
        if let view = view {
            view.toggleFlash()
        }
    }
    
    
    func hideInstructions(hidden : Bool) {
        if let view = view {
            view.hideInstructions(hidden)
        }
    }
    
    func setTextForCount(count : Int) {
        if let view = view {
            view.setTextForCount(count)
        }
    }
    
    func ownsWarp(owns : Bool) {
        if let ui = ui {
            ui.ownsWarp(owns)
        }
    }
    
	var window: UIWindow?
    var storyboard: UIStoryboard?
    let syncro = Syncro()
    var ui : CaptureController?
    var view : OpenGLView?
    
    
    var downloadRequest: Alamofire.Request!
    
    let userHasOnboardedKey = "user_has_onboarded1"

	func application(application: UIApplication, didFinishLaunchingWithOptions launchOptions: [NSObject : AnyObject]?) -> Bool {

        let nc = NSNotificationCenter.defaultCenter()
        
        let qualityOfServiceClass = QOS_CLASS_BACKGROUND
        let backgroundQueue = dispatch_get_global_queue(qualityOfServiceClass, 0)
        dispatch_async(backgroundQueue, {
            //let facemarksURL = "https://s3-us-west-1.amazonaws.com/requestlabs/apps/maskito
            // facemarks.dat.zip"
            
            let datURL = "https://s3-us-west-1.amazonaws.com/requestlabs/apps/maskito/facemarks.dat"
            
            var localPath: NSURL?
            var progress: String = ""
            
            let documentsUrl =  NSFileManager.defaultManager().URLsForDirectory(.DocumentDirectory, inDomains: .UserDomainMask).first! as NSURL
            
            let datUrl = documentsUrl.URLByAppendingPathComponent(NSURL(string: datURL)!.lastPathComponent!)
            
            if NSFileManager().fileExistsAtPath(datUrl.path!) {
                // the file exists woohoo
                print("the file exists")

                User.completeDownloaded()
            } else {
                // the file needs to be downloaded and fast!!
                
                if self.downloadRequest == nil {
                    print("nil starting download")
                    self.downloadRequest = Alamofire.download(.GET,
                        datURL,
                        destination: { (temporaryURL, response) in
                            let pathComponent = response.suggestedFilename
                            localPath = documentsUrl.URLByAppendingPathComponent(pathComponent!)
                            return localPath!
                    })
                } else {
                    print("resuming")
                    self.downloadRequest.resume()
                }
                
                    
                self.downloadRequest!.response { (request, response, _, error) in
                        print("Downloaded file to \(localPath!)")
                        nc.postNotificationName("FileExists", object: nil)
                        User.completeDownloaded()
                        NSUserDefaults.standardUserDefaults().setBool(true, forKey: "facemarks_downloaded")
                } // end response
                
                self.downloadRequest!.progress { bytesRead, totalBytesRead, totalBytesExpectedToRead in
                        let currentProgress = String(Int64(Float(totalBytesRead) / Float(totalBytesExpectedToRead) * 100))
                        if currentProgress != progress {
                            progress = currentProgress
                            NSUserDefaults.standardUserDefaults().setValue(progress, forKey: "facemarks_progress")
                        } // end if
                } // end progress
            } // end else
        })
        
        // setup Storyboad
        storyboard = UIStoryboard(name: "Main", bundle: nil)
        
        // Override point for customization after application launch.
        UIApplication.sharedApplication().statusBarStyle = .LightContent
        
        Analytics.info("APP-OPENED")
        
        
        //
        Nav.handle()
        //
		return true
    }

    func completeDownload() {
        
    }

	func applicationWillResignActive(application: UIApplication) {
		// Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
		// Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
	}

	func applicationDidEnterBackground(application: UIApplication) {
		// Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
		// If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
        Analytics.info("APP-DID-ENTER-BACKGROUND")
	}

	func applicationWillEnterForeground(application: UIApplication) {
		// Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
	}

	func applicationDidBecomeActive(application: UIApplication) {
		// Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
        Analytics.info("APP-ACTIVE")
	}

	func applicationWillTerminate(application: UIApplication) {
		// Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
		// Saves changes in the application's managed object context before the application terminates.
		//self.saveContext()
        Analytics.info("APP-DID-TERMINATE")
	}

}

