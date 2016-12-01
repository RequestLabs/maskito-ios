//
//  Nav.swift
//  Maskito
//
//  Created by Gareth on 5/16/16.
//
//

import Foundation
import UIKit


class Nav : NSObject {

    
    class func getOnboarding() -> UIViewController? {
        let storyboard = UIStoryboard(name: "Onboarding", bundle: nil)
        let onboardingFlow = storyboard.instantiateViewControllerWithIdentifier("onboardingFlow") as! OnboardingMasterViewController
        let screen1 = storyboard.instantiateViewControllerWithIdentifier("screen1")
        //let screen2 = storyboard.instantiateViewControllerWithIdentifier("screen2")
        //let screen3 = storyboard.instantiateViewControllerWithIdentifier("screen3")
        //let screen4 = storyboard.instantiateViewControllerWithIdentifier("screen4")
        onboardingFlow.addViewController(screen1)
        //onboardingFlow.addViewController(screen2)
        //onboardingFlow.addViewController(screen3)
        //onboardingFlow.addViewController(screen4)
        return onboardingFlow
    }
    
    class func getAuthboarding() -> UIViewController? {
        let storyboard = UIStoryboard(name: "Authentication", bundle: nil)
        let authVC = storyboard.instantiateInitialViewController()
        return authVC
    }
    
    class func getDownloading() -> UIViewController? {
        let storyboard = UIStoryboard(name: "Assets", bundle: nil)
        let authVC = storyboard.instantiateInitialViewController()
        return authVC
    }
    
    class func getNormal() -> UIViewController? {
        let storyboard = UIStoryboard(name: "Main", bundle: nil)
        let initialVC = storyboard.instantiateInitialViewController()
        return initialVC
    }
    
    class func handle() -> Void {
        
        let appDelegate = UIApplication.sharedApplication().delegate as! AppDelegate
        print(User)
        if !User.hasOnboarded() {
            print("userhasnot onboarded")
            appDelegate.window?.rootViewController = getOnboarding()
            
        } else if !User.hasAuthed() {
            print("userhasnot authed")
            appDelegate.window?.rootViewController = getAuthboarding()
        } else if !User.hasDownloaded() {
            appDelegate.window?.rootViewController = getDownloading()
        } else {
            print("userhas onboarded and authed")
            appDelegate.window?.rootViewController = getNormal()
        }
        
        
    }
}