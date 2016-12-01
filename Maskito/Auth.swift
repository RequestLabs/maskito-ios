//
//  Auth.swift
//  Maskito
//
//  Created by Gareth on 5/16/16.
//
//

import Foundation
import Alamofire

class Auth : NSObject {
    
    class func signUp(email: String, password: String, completionHandler : ((isValid : Bool) -> Void)) {
        let url = "http://localhost:8080/app/maskito/signup"
        let parameters = ["email": email, "password": password]
        Alamofire.request(.POST, url, parameters: parameters).responseJSON
            { response in switch response.result {
            case .Success(let JSON):
                print("Success with JSON: \(JSON)")
                let response = JSON as! NSDictionary
                //example if there is an id
                let valid = response.objectForKey("valid") as! Bool
                
                if valid == true {
                    User.setEmail(email)
                }
                
                return completionHandler(isValid : valid)
                
            case .Failure(let error):
                print("Request failed with error: \(error)")
                return completionHandler(isValid: false)
                }
        }
    }
    
    class func login(email: String, password: String, completionHandler: ((isValid: Bool) -> Void)) {
        let url = "http://localhost:8080/app/maskito/login"
        let parameters = ["email": email, "password": password]
        Alamofire.request(.POST, url, parameters: parameters).responseJSON
            { response in switch response.result {
            case .Success(let JSON):
                print("Success with JSON: \(JSON)")
                let response = JSON as! NSDictionary
                //example if there is an id
                let valid = response.objectForKey("valid") as! Bool
                //let name = User.getName()
                if valid == true {
                    User.setEmail(email)
                    //User.setName(name)
                }
                
                return completionHandler(isValid : valid)
                
            case .Failure(let error):
                print("Request failed with error: \(error)")
                return completionHandler(isValid: false)
                }
        }
    }
    
    class func newName(name: String, completionHandler: ((isValid: Bool) -> Void)) {
        let url = "http://localhost:8080/app/maskito/name"
        let email = User.getEmail()
        let parameters = ["email": email, "name": name]
        Alamofire.request(.POST, url, parameters: parameters).responseJSON
            { response in switch response.result {
            case .Success(let JSON):
                print("Success with JSON: \(JSON)")
                let response = JSON as! NSDictionary
                //example if there is an id
                let valid = response.objectForKey("valid") as! Bool
                
                if valid == true {
                    User.setName(name)
                }
                
                return completionHandler(isValid : valid)
                
            case .Failure(let error):
                print("Request failed with error: \(error)")
                return completionHandler(isValid: false)
                }
        }
    }
    
    class func checkName(name: String, completionHandler: ((isValid: Bool) -> Void)) {
        let url = "http://localhost:8080/app/maskito/name/check"
        let email = User.getEmail()
        let parameters = ["email": email, "name": name]
        Alamofire.request(.POST, url, parameters: parameters).responseJSON
            { response in switch response.result {
            case .Success(let JSON):
                print("Success with JSON: \(JSON)")
                let response = JSON as! NSDictionary
                //example if there is an id
                let valid = response.objectForKey("valid") as! Bool
                return completionHandler(isValid : valid)
            case .Failure(let error):
                print("Request failed with error: \(error)")
                return completionHandler(isValid: false)
                }
        }
    }

}
