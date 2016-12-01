//
//  AuthFindFriendsViewController.swift
//  Maskito
//
//  Created by Gareth on 5/14/16.
//
//

import UIKit

class AuthFindFriendsViewController: UIViewController {

    var name: String!
 
    @IBOutlet weak var infoLabel: UILabel!
    override func viewDidLoad() {
        super.viewDidLoad()
        
        infoLabel.text = "From now on you can be known as " + "\(name)" + " on Maskito"
        Analytics.view("FindFriendsViewController")
        // Do any additional setup after loading the view.
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    @IBAction func findFriendsBtn(sender: AnyObject) {
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
