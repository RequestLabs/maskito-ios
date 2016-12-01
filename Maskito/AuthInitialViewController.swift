//
//  AuthInitialViewController.swift
//  Maskito
//
//  Created by Gareth on 5/14/16.
//
//

import UIKit

class AuthInitialViewController: UIViewController {

    override func viewDidLoad() {
        super.viewDidLoad()
        
        setupNav()
        
        Analytics.view("AuthInitialViewController")

        // Do any additional setup after loading the view.
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    
    override func viewDidDisappear(animated: Bool) {
        super.viewDidDisappear(animated)
        self.navigationController?.setNavigationBarHidden(false, animated: false)
    }
    
    func setupNav() {
        self.navigationController?.setNavigationBarHidden(true, animated: true)
    }

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
        // Get the new view controller using segue.destinationViewController.
        // Pass the selected object to the new view controller.
    }
    */

}
