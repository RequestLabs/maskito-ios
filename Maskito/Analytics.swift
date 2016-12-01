//
//  Analytics.swift
//

import Foundation

class Analytics : NSObject {
    class func install(logLevel: GAILogLevel = GAILogLevel.None) {
        // Configure tracker from GoogleService-Info.plist.
        var configureError:NSError?
        GGLContext.sharedInstance().configureWithError(&configureError)
        assert(configureError == nil, "Error configuring Google services: \(configureError)")
        
        // Optional: configure GAI options.
        let gai = GAI.sharedInstance()
        gai.trackUncaughtExceptions = true  // report uncaught exceptions
        gai.logger.logLevel = logLevel // remove before app release
    }
    class func view(view: String) {
        let tracker = GAI.sharedInstance().defaultTracker
        if tracker != nil {
            tracker.set(kGAIDescription, value: view)
            let builder: NSObject = GAIDictionaryBuilder.createScreenView().build()
            tracker.send(builder as! [NSObject : AnyObject])
        }

    }
    
    class func push(action: String) {
        let tracker = GAI.sharedInstance().defaultTracker
        if tracker != nil {
            let builder: NSObject = GAIDictionaryBuilder.createEventWithCategory(
                "actions",
                action: "push",
                label: action,
                value: nil).build()
             tracker.send(builder as! [NSObject : AnyObject])
        }


    }
    
    class func info(action: String) {
        let tracker = GAI.sharedInstance().defaultTracker
        let builder: NSObject = GAIDictionaryBuilder.createEventWithCategory(
            "actions",
            action: "app",
            label: action,
            value: nil).build()
        if tracker != nil {
            tracker.send(builder as! [NSObject : AnyObject])
        }
    }
}
