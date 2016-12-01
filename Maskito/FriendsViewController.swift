//
//  FriendsViewController.swift
//

import UIKit

class FriendsViewController: UIViewController, UITableViewDelegate, UITableViewDataSource {

    @IBOutlet weak var tableView: UITableView!
    
     var items: [String] = ["We", "Heart", "You"]
    
    override func viewDidLoad() {
        super.viewDidLoad()

        var nib = UINib(nibName: "FriendsTableViewCell", bundle: nil)
        //self.tableView.registerClass(UITableViewCell.self, forCellReuseIdentifier: "cell")
        self.tableView.registerNib(nib, forCellReuseIdentifier: "customCell")
        
        // fix navigation
        let navHeight = CGFloat(100)
        let frame = CGRectMake(0, 0, UIScreen.mainScreen().bounds.width, navHeight)
        self.navigationController?.navigationBar.frame = frame
        
        // Do any additional setup after loading the view.
    }
    
    func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return self.items.count
    }
    
    func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        let cell:FriendsTableViewCell = self.tableView.dequeueReusableCellWithIdentifier("customCell")! as! FriendsTableViewCell
        
        
        cell.loadItem("test", image: "http://localhost:8080/app/maskito/profile?user_id=gareth")
        
        return cell
       
        
        
    }
    
    func tableView(tableView: UITableView, heightForRowAtIndexPath indexPath: NSIndexPath) -> CGFloat
    {
        return 60.0;//Choose your custom row height
    }
    
    
    func tableView(tableView: UITableView, didSelectRowAtIndexPath indexPath: NSIndexPath) {
        
    }
    
    

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
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
