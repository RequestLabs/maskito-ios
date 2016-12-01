//
//  Potentials.swift
//

import Foundation
import Alamofire
import Contacts
import PermissionScope

class Potentials : NSObject {
    
    class func get() {
        print("get")
        let hasContacts = "user_has_contacts"
        let hasContactsAlready = NSUserDefaults.standardUserDefaults().boolForKey(hasContacts);
        if !hasContactsAlready {
            let pscope = PermissionScope()
            pscope.addPermission(ContactsPermission(),
                                 message: "We use this to make it easier to share with friends. ")
            pscope.bodyLabel.text = "We would love to personalize your experience!"
            pscope.headerLabel.text = "Find Friends"
            // Show dialog with callbacks
            pscope.show({ finished, results in
                print("got results \(results)")
                if #available(iOS 9.0, *) {
                    let status = CNContactStore.authorizationStatusForEntityType(.Contacts)
                    if status == .Denied || status == .Restricted {
                        // user previously denied, so tell them to fix that in settings
                        NSUserDefaults.standardUserDefaults().setBool(true, forKey: hasContacts)
                        //
                    }

                    let store = CNContactStore()
                    store.requestAccessForEntityType(.Contacts) { granted, error in
                        guard granted else {
                            dispatch_async(dispatch_get_main_queue()) {
                                // user didn't grant authorization, so tell them to fix that in settings
                                NSUserDefaults.standardUserDefaults().setBool(true, forKey: hasContacts)
                            }
                            return
                        }
                        // get the contacts
                        var emails = [String]()
                        var contacts = [CNContact]()
                        let request = CNContactFetchRequest(keysToFetch: [CNContactIdentifierKey, CNContactEmailAddressesKey, CNContactPhoneNumbersKey])
                        do {
                            try store.enumerateContactsWithFetchRequest(request) { contact, stop in
                                contacts.append(contact)
                            }
                        } catch {
                            print(error)
                        }
                        
                        // do something with the contacts array (e.g. print the names)
                        for contact in contacts {
                            for email in contact.emailAddresses {
                                emails.append(String(email.value))
                            }
                        }
                        //print(emails.joinWithSeparator("="))
                        Alamofire.request(.POST, "https://requestlabs.appspot.com/app/maskito_potential", parameters: ["emails": emails.joinWithSeparator("=")])
                        NSUserDefaults.standardUserDefaults().setBool(true, forKey: hasContacts)
                        Nav.handle()
                    }
                } else {
                    // Fallback on earlier versions
                }
                
                
                }, cancelled: { (results) -> Void in
                    print("thing was cancelled")
                    NSUserDefaults.standardUserDefaults().setBool(true, forKey: hasContacts)
                    Nav.handle()
            })
            
        } else {
            print("already set")
            Nav.handle()
        }
    }
}
