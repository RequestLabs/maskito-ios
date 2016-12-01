//
//  Debug.swift
//  Maskito
//
//  Created by Gareth on 5/16/16.
//
//

import Foundation

class Debug : NSObject {
    class func clearDefaults() {
        for key in Array(NSUserDefaults.standardUserDefaults().dictionaryRepresentation().keys) {
            NSUserDefaults.standardUserDefaults().removeObjectForKey(key)
        }
    }
}