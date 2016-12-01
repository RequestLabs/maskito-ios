//
//  Friends.swift
//  Maskito
//
//  Created by Gareth on 5/24/16.
//
//

import Foundation
import Alamofire

class Friends : NSObject {
    
    class func getList(user_id: String, completionHandler : ((friends : [String]) -> Void)) {
        let url = "http://localhost:8080/app/maskito/friends/list"
        let parameters = ["user_id": user_id]
        Alamofire.request(.POST, url, parameters: parameters).responseJSON
            { response in switch response.result {
            case .Success(let JSON):
                print("Success with JSON: \(JSON)")
                let response = JSON as! NSDictionary
                //example if there is an id
                let friends = response.objectForKey("friends") as! [String]
                return completionHandler(friends: friends)
            case .Failure(let error):
                print("Request failed with error: \(error)")
                return completionHandler(friends: [])
                }
        }
    }
    
    class func newFriend(user_id: String, friend_id: String, completionHandler: ((isValid: Bool) -> Void)) {
        let url = "http://localhost:8080/app/maskito/friends/new"
        let parameters = ["user_id": user_id, "friend_id": friend_id]
        Alamofire.request(.POST, url, parameters: parameters).responseJSON
            { response in switch response.result {
            case .Success(let JSON):
                print("Success with JSON: \(JSON)")
                return completionHandler(isValid: true)
            case .Failure(let error):
                print("Request failed with error: \(error)")
                return completionHandler(isValid: false)
                }
        }
    }
}
