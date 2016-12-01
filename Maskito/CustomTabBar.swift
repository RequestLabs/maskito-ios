//
//  CustomTabBar.swift
//
//

import Foundation

class CustomTabBar: UITabBar {

    override func sizeThatFits(size: CGSize) -> CGSize {
        var sizeThatFits = super.sizeThatFits(size)
        sizeThatFits.height = 60
        
        return sizeThatFits
    }
    

}
