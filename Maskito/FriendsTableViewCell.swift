//
//  FriendsTableViewCell.swift
//  Maskito
//
//  Created by Gareth on 5/24/16.
//
//

import UIKit

class FriendsTableViewCell: UITableViewCell {

    @IBOutlet weak var profilePic: UIImageView!
    @IBOutlet weak var userName: UILabel!
    
    override func awakeFromNib() {
        super.awakeFromNib()
        // Initialization code
    }
    
    func loadItem(user: String, image: String) {
        //profilePic.image = UIImage(named: image)
        let profilePicture = UIImage(data: NSData(contentsOfURL: NSURL(string:image)!)!)!
        profilePicture.rounded
        profilePicture.circle
        profilePic.image = profilePicture.rounded
        userName.text = user
}
    

    override func setSelected(selected: Bool, animated: Bool) {
        super.setSelected(selected, animated: animated)

        // Configure the view for the selected state
    }
    
}
