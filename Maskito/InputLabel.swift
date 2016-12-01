//
//  InputLabel.swift
//
//

import Foundation
import UIKit

@IBDesignable
class InputLabel: UITextField {
    
    @IBInspectable var insetX: CGFloat = 0
    @IBInspectable var insetY: CGFloat = 0
    
    override func drawTextInRect(rect: CGRect) {
        var bounds = rect
        bounds.origin.x += 20;
        let insets = UIEdgeInsets.init(top:insetX, left: insetY, bottom: 0, right: 20)
        super.drawTextInRect(UIEdgeInsetsInsetRect(bounds, insets))
    }
    
    override func editingRectForBounds(rect: CGRect) -> CGRect {
        var bounds = rect
        bounds.origin.x += 20;
        let insets = UIEdgeInsets.init(top: insetX, left: 50, bottom: 0, right: 20)
        return UIEdgeInsetsInsetRect(bounds, insets)
    }
    
    override func leftViewRectForBounds(bounds: CGRect) -> CGRect {
        let leftBounds = CGRectMake(22, 10, 30, 30)
        return leftBounds
    }
}
