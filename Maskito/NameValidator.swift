//
//  NameValidator.swift
//

import Foundation
import UIKit

extension String {
    var isNameValid: Bool {
        // 8 character length
        if characters.count < 3 { return false }
        return true
    }
}
