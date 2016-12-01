//
//  SignUpViewController.swift
//  Maskito
//
//  Created by Gareth on 5/14/16.
//
//

import UIKit
import TTTAttributedLabel

class SignUpViewController: UIViewController, TTTAttributedLabelDelegate, UITextFieldDelegate, UITextViewDelegate {

    @IBOutlet weak var emailAddress: InputLabel!
    @IBOutlet weak var password: InputLabel!

    @IBOutlet weak var terms: TTTAttributedLabel!
    
    let linkColor = UIColor(hexString: "#EAC4AB")
    
    var activeInput = InputLabel()
    
    let infoStr = "By using this app you agree to our Terms and Conditions and Privacy Policy."
    let termsAndConditionsURL = "http://www.example.com/terms";
    let privacyURL = "http://www.example.com/privacy";

    
    override func viewDidLoad() {
        super.viewDidLoad()
        setupNav()
        emailAddress.delegate = self
        password.delegate = self
        
        emailAddress.keyboardType = UIKeyboardType.EmailAddress
        emailAddress.returnKeyType = UIReturnKeyType.Next
        
        password.returnKeyType = UIReturnKeyType.Done
        
        emailAddress.tag = 0
        password.tag = 1
        
        activeInput.delegate = self
        
        setupEmail()
        setupPassword()
        
        emailAddress.tintColor = UIColor(hexString: "#2F346A")
        password.tintColor = UIColor(hexString: "#2F346A")
        
        setupTerms()
        
        emailAddress.addTarget(self, action: #selector(SignUpViewController.emailAddressDidChange(_:)), forControlEvents: UIControlEvents.EditingChanged)
        password.addTarget(self, action: #selector(SignUpViewController.passwordDidChange(_:)), forControlEvents: UIControlEvents.EditingChanged)
        
        
        // Do any additional setup after loading the view.
        Analytics.view("SignUpViewController")
    }
    
    func setupTerms() {
        terms.delegate = self
        terms.text = infoStr
        terms.linkAttributes = [NSForegroundColorAttributeName : linkColor]
        terms.activeLinkAttributes = [NSForegroundColorAttributeName : linkColor]
        
        let termsRange: NSRange = (infoStr as NSString).rangeOfString("Terms and Conditions")
        self.terms.addLinkToURL(NSURL(string:"https://requestlabs.appspot.com/app/maskito/terms"), withRange:termsRange)
        
        let policyRange: NSRange = (infoStr as NSString).rangeOfString("Privacy Policy")
        self.terms.addLinkToURL(NSURL(string:"https://requestlabs.appspot.com/app/maskito/privacy"), withRange:policyRange)
        
    }
    
    func emailAddressDidChange(textField: UITextField) {
        if !(emailAddress.text?.isValidEmail())! {
            setupEmail("emailLogoRed")
        } else {
            setupEmail()
        }
    }
    
    func passwordDidChange(textField: UITextField) {
        password.secureTextEntry = true
        if !(password.text?.isValidPassword)! {
            setupPassword("lockLogoRed")
        } else {
            setupPassword()
        }
    }
    
    func setupEmail(imageName: String = "emailLogo") {
        let imageView = UIImageView();
        imageView.frame = CGRect(x: 20, y: 5, width: emailAddress.frame.height-10, height: emailAddress.frame.height-10)
        let image = UIImage(named: imageName)
        image?.imageWithRenderingMode(UIImageRenderingMode.AlwaysTemplate)
        imageView.image = image;
        imageView.clipsToBounds = true
        imageView.contentMode = UIViewContentMode.ScaleAspectFit
        imageView.tintColor = UIColor.redColor()
        emailAddress.leftView = imageView
        emailAddress.leftViewMode = UITextFieldViewMode.Always
        
    }
    
    
    func attributedLabel(label: TTTAttributedLabel!, didSelectLinkWithURL url: NSURL!) {
        //
        UIApplication.sharedApplication().openURL(url)
    }
    
    func setupPassword(imageName: String = "lockLogo") {
        let imageView = UIImageView();
        imageView.frame = CGRect(x: 20, y: 5, width: password.frame.height-10, height: password.frame.height-10)
        let image = UIImage(named: imageName);
        imageView.image = image;
        imageView.clipsToBounds = true
        imageView.contentMode = UIViewContentMode.ScaleAspectFit
        password.leftView = imageView
        password.leftViewMode = UITextFieldViewMode.Always
    }
    
    
    
    func textFieldDidBeginEditing(textField: UITextField) {
        textField.text = ""
        
    }
    
    
    override func viewDidAppear(animated: Bool) {
        super.viewDidAppear(animated)
        
        setupNav()
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    func showPasswordAlert() {
        print("showPassword Alert")
        let alertController = UIAlertController(title: "Invalid Password", message:
            "A password requires 8 characters, which should include at least 1 upper, 1 lower and 1 numerical character.", preferredStyle: UIAlertControllerStyle.Alert)
        alertController.addAction(UIAlertAction(title: "Dismiss", style: UIAlertActionStyle.Default,handler: nil))
        
        self.presentViewController(alertController, animated: true, completion: nil)
    }
    
    func showEmailAlert() {
        let alertController = UIAlertController(title: "Invalid Email", message:
            "Email addresss must be entered in the correct format e.g. tom@google.com", preferredStyle: UIAlertControllerStyle.Alert)
        alertController.addAction(UIAlertAction(title: "Dismiss", style: UIAlertActionStyle.Default,handler: nil))
        
        self.presentViewController(alertController, animated: true, completion: nil)
    }
    
    func showSubmitAlert() {
        let alertController = UIAlertController(title: "Invalid Email", message:
            "Oops something went wrong, perhaps you already have an account with us.", preferredStyle: UIAlertControllerStyle.Alert)
        alertController.addAction(UIAlertAction(title: "Dismiss", style: UIAlertActionStyle.Default,handler: nil))
        
        self.presentViewController(alertController, animated: true, completion: nil)
    }
    
    func textFieldShouldReturn(textField: UITextField) -> Bool {
        let nextTage=textField.tag+1
        let nextResponder=textField.superview?.viewWithTag(nextTage) as UIResponder!
       
        if (nextResponder != nil){
            // Found next responder, so set it.
            // this is email
            nextResponder?.becomeFirstResponder()
        }
        else
        {
            // Send SignUp Request
            var error = false
            if !(password.text?.isValidPassword)! {
                error = true
                showPasswordAlert()
            }
            if !(emailAddress.text?.isValidEmail())! {
                error = true
                showEmailAlert()
            }
            
            if !error {
                print("done")
                //
                Auth.signUp(emailAddress!.text!, password: password!.text!, completionHandler: { (isValid) in
                    if isValid {
                        self.performSegueWithIdentifier("selectName", sender: nil)
                    } else {
                        self.showSubmitAlert()
                    }
                })
            }
        }
        return true
    }
    
    
    func setupNav() {
        self.navigationController?.setNavigationBarHidden(false, animated: false)
        let navbar = self.navigationController?.navigationBar
        navbar?.titleTextAttributes = [NSForegroundColorAttributeName: UIColor.whiteColor()]
        self.navigationController?.navigationBar.backIndicatorImage = UIImage(named: "backBtn")
        self.navigationController?.navigationBar.backIndicatorTransitionMaskImage = UIImage(named: "backBtn")
    }
    
    
    override func viewWillDisappear(animated : Bool) {
        super.viewWillDisappear(animated)
        self.navigationController?.setNavigationBarHidden(true, animated: false)
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
