//
//  User.swift
//  Maskito
//
//  Created by Gareth on 5/16/16.
//
//

import Foundation
import Alamofire

class User : NSObject {
    
    class func hasOnboarded() -> Bool {
        return NSUserDefaults.standardUserDefaults().boolForKey("user_has_onboarded");
    }
    
    class func completeOnboarding() -> Void {
        NSUserDefaults.standardUserDefaults().setBool(true, forKey: "user_has_onboarded")
    }
    
    class func hasDownloaded() -> Bool {
        return NSUserDefaults.standardUserDefaults().boolForKey("user_has_downloaded");
    }
    
    class func completeDownloaded() -> Void {
        NSUserDefaults.standardUserDefaults().setBool(true, forKey: "user_has_downloaded")
    }
    
    
    class func hasAuthed() -> Bool {
        return NSUserDefaults.standardUserDefaults().boolForKey("user_has_auth");
    }
    
    class func completeAuthorization() -> Void {
        NSUserDefaults.standardUserDefaults().setBool(true, forKey: "user_has_auth")
    }
    
    class func getEmail() -> String {
        let email = NSUserDefaults.standardUserDefaults().stringForKey("email")
        return email!
    }
    
    class func setEmail(email: String) {
        NSUserDefaults.standardUserDefaults().setValue(email, forKey: "email")
    }
    
    class func getName() -> String {
        let name = NSUserDefaults.standardUserDefaults().stringForKey("name")
        return name!
    }
    
    class func setName(name: String) {
        NSUserDefaults.standardUserDefaults().setValue(name, forKey: "name")
    }
    
}