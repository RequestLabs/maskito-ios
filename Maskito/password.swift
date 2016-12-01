//
//  password.swift
//
import Foundation
import UIKit

extension String {
    var isValidPassword: Bool {
        // 8 character length
        if characters.count < 8 { return false }
        // at least 1 alpha
        if rangeOfCharacterFromSet(.letterCharacterSet(), options: .LiteralSearch, range: nil) == nil { return false }
        // at least 1 numeric
        if rangeOfCharacterFromSet(.decimalDigitCharacterSet(), options: .LiteralSearch, range: nil) == nil { return false }
        // check uppercase letter
        if rangeOfCharacterFromSet(NSCharacterSet.uppercaseLetterCharacterSet(), options: .LiteralSearch, range: nil) == nil { return false }
        // check lowercase letter
        if rangeOfCharacterFromSet(NSCharacterSet.lowercaseLetterCharacterSet(), options: .LiteralSearch, range: nil) == nil { return false }
        
        return true
    }
}
