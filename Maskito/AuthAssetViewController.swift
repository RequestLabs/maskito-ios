//
//  AuthAssetViewController.swift
//  Maskito
//
//  Created by Gareth on 6/7/16.
//
//

import Foundation
import UIKit

class AuthAssetViewController: UIViewController {
    
    @IBOutlet weak var goBtn: UIButton!
    @IBOutlet weak var progressLabel: UILabel!
    
    var progressTimer: NSTimer!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let nc = NSNotificationCenter.defaultCenter()
        nc.addObserver(self, selector: #selector(userAuthenticated), name: "FilesExists", object: nil)
        
        progressTimer = NSTimer.scheduledTimerWithTimeInterval(2.5, target: self, selector: Selector("progressUpdater"), userInfo: nil, repeats: true)


        Analytics.view("AuthAssetViewController")
        // Do any additional setup after loading the view.
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    func userAuthenticated() {
        self.progressTimer.invalidate()
        self.progressTimer = nil
        
        User.completeOnboarding()
        Nav.handle()
    }
    
    
    func progressUpdater() {
        let progress = NSUserDefaults.standardUserDefaults().stringForKey("facemarks_progress")
        progressLabel.text = progress! + "%"
        
        if progress == "100" {
            userAuthenticated()
        }
    }
    
    
        

    
    @IBAction func userAuthed(sender: AnyObject) {
        User.completeAuthorization()
        //Potentials.get()
        Nav.handle()
    
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