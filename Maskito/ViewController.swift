//
//  ViewController.swift
//

import UIKit

class ViewController: UIViewController {
    
	override func viewDidLoad() {
		super.viewDidLoad()
        print("in view controller")
        self.performSegueWithIdentifier("activeUser", sender: nil)
	}

	override func didReceiveMemoryWarning() {
		super.didReceiveMemoryWarning()
		// Dispose of any resources that can be recreated.
	}
}

