//
//  email.swift
//  Maskito
//
//  Created by Gareth on 5/14/16.
//
//

import Foundation
import UIKit

extension String {
    func isValidEmail() -> Bool {
        do {
            let emailRegex = "[A-Z0-9a-z._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,6}"
            return NSPredicate(format: "SELF MATCHES %@", emailRegex).evaluateWithObject(self)
        } catch {
            return false
        }
    }
}