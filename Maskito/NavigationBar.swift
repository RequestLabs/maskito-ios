//
//  NavigationBar.swift
//  Maskito
//
//  Created by Gareth on 5/24/16.
//
//

import UIKit

class NavigationBar: UINavigationBar {

    /*
    // Only override drawRect: if you perform custom drawing.
    // An empty implementation adversely affects performance during animation.
    override func drawRect(rect: CGRect) {
        // Drawing code
    }
    */
    
    override func sizeThatFits(size: CGSize) -> CGSize {
        let newSize:CGSize = CGSizeMake(self.superview!.frame.size.width, 80)
        return newSize
    }
    

}
