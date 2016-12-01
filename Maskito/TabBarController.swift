//
//  TabBarController.swift
//

import Foundation
import UIKit

class TabBarController: UITabBarController {
    override func viewDidLoad() {
        super.viewDidLoad()
        self.tabBar.tintColor = UIColor(hexString: "#2F346A")
        self.tabBar.backgroundColor = UIColor.whiteColor()
        // Do any additional setup after loading the view.
        //self.tabBar.shadowImage = UIImage()
        //self.tabBar.backgroundImage = UIImage()
        
        let topBorder = CALayer()
        topBorder.frame = CGRectMake(0,0, self.view.frame.width, 1)
        topBorder.backgroundColor = UIColor(hexString: "#2F346A").CGColor
        self.tabBar.layer.addSublayer(topBorder)
        
        for tab in self.tabBar.items! {
            //if selectedIndex != self.tabBar.items?.indexOf(tab) {
            if let image = tab.image {
                tab.image = image.imageWithColor(UIColor(hexString:"#D9D9DA")).imageWithRenderingMode(.AlwaysOriginal)

            }
        }

    
    }

    func hidden() {
        moveWithAnimation(self.tabBar, height: -self.tabBar.bounds.height)
    }
    
    func moveWithAnimation(tabBar:UITabBar, height:CGFloat) {
        UIView.animateWithDuration(1, animations: { () -> Void in
            self.moveViewTo(tabBar, deltaY: height)
        })
    }
    
    func moveViewTo(view:UIView, deltaY:CGFloat) {
        var bounds = view.bounds
        bounds.origin.y = deltaY
        view.bounds = bounds
    }
    
    override func tabBar(tabBar: UITabBar, didSelectItem item: UITabBarItem)
    {
        let items = tabBar.items
        let index = items?.indexOf(item)
        if index == 1 {
            self.tabBar.hidden = true
        }
    }
    
}
