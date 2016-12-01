//
//  AuthNewUserViewController.swift
//  Maskito
//
//  Created by Gareth on 5/14/16.
//
//

import UIKit

class AuthNewUserViewController: UIViewController, UITextFieldDelegate {

    @IBOutlet weak var nameField: InputLabel!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        nameField.delegate = self
        nameField.becomeFirstResponder()

        nameField.addTarget(self, action: #selector(AuthNewUserViewController.nameFieldDidChange(_:)), forControlEvents: UIControlEvents.EditingChanged)
        
        nameField.returnKeyType = UIReturnKeyType.Done
        nameField.tintColor = UIColor(hexString: "#2F346A")
        
        setupName()
        // Do any additional setup after loading the view.
        Analytics.view("AuthNewUserViewController")
    }
    
    func nameFieldDidChange(textField: UITextField) {
        // check the name exists or not
        if textField.text?.isNameValid == true {
            
            Auth.checkName(textField.text!, completionHandler: { (isValid) in
                if !isValid {
                    self.setupName("tickLogoGreen")
                } else {
                    self.setupName("crossLogoRed")
                }
            })
            
        } else {
            setupName("crossLogoRed")
        }
        // names must be 3 characters or more
        
    }

    func textFieldDidBeginEditing(textField: UITextField) {
        nameField.text = ""
    }
    
    func textFieldShouldReturn(textField: UITextField) -> Bool {
        if nameField.text!.isNameValid == false{
            showError()
        } else {
            print("valid name")
            // Send Auth Request
            Auth.newName(nameField.text!, completionHandler: { (isValid) in
                // 
                if isValid {
                    self.performSegueWithIdentifier("findFriends", sender: nil)
                } else {
                    print("show generic error")
                }
            })
        }
        return true
    }
    
    
    
    func showError() {
        let alertController = UIAlertController(title: "Oops", message:
            "Looks like we have an issue with your desired name. Your name needs to be at least 3 characters in length and unique.", preferredStyle: UIAlertControllerStyle.Alert)
        alertController.addAction(UIAlertAction(title: "Dismiss", style: UIAlertActionStyle.Default,handler: nil))
        
        self.presentViewController(alertController, animated: true, completion: nil)
    }
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    func setupName(imageName: String = "crossLogoRed") {
        let imageView = UIImageView();
        imageView.frame = CGRect(x: 20, y: -5, width: nameField.frame.width-10, height: nameField.frame.height-10)
        let image = UIImage(named: imageName)
        image?.imageWithRenderingMode(UIImageRenderingMode.AlwaysTemplate)
        imageView.image = image;
        imageView.clipsToBounds = true
        imageView.contentMode = UIViewContentMode.ScaleAspectFit
        imageView.tintColor = UIColor.redColor()
        nameField.leftView = imageView
        nameField.leftViewMode = UITextFieldViewMode.Always
    }

    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
        // Get the new view controller using segue.destinationViewController.
        // Pass the selected object to the new view controller.
        if segue.identifier == "findFriends" {
            if let nextVC = segue.destinationViewController as? AuthFindFriendsViewController {
                nextVC.name = nameField.text
            }
            
        }
    }
    

}
