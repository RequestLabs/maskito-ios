//
//  ControlViewController.swift
//  FaceWarpApp
//
//  Created by Thomas Nickson on 01/10/2015.
//  Copyright © 2015 Phi Research. All rights reserved.
//

import Foundation
import UIKit

class ControlViewController : UIViewController, UIPickerViewDelegate, UIPickerViewDataSource {
    
    @IBOutlet weak var picker: UIPickerView!
    var pickerData: [String] = [String]()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
        // Connect data:
        self.picker.delegate = self
        self.picker.dataSource = self
        pickerData = ["Silly", "Wibbly", "Beautify", "TinyFace"]
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    // The number of columns of data
    func numberOfComponentsInPickerView(pickerView: UIPickerView) -> Int {
        return 1
    }
    
    // The number of rows of data
    func pickerView(pickerView: UIPickerView, numberOfRowsInComponent component: Int) -> Int {
        return pickerData.count
    }
    
//    // The data to return for the row and component (column) that's being passed in
//    func pickerView(pickerView: UIPickerView, titleForRow row: Int, forComponent component: Int) -> String? {
//        return pickerData[row]
//    }
    func pickerView(pickerView: UIPickerView, attributedTitleForRow row: Int, forComponent component: Int) -> NSAttributedString? {
        let string = pickerData[row]
        return NSAttributedString(string: string, attributes: [NSForegroundColorAttributeName:UIColor.whiteColor()])
    }
}
