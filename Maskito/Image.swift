//
//  Image.swift
//

import Foundation

extension UIImage {
    func imageWithColor(tintColor: UIColor) -> UIImage {
        UIGraphicsBeginImageContextWithOptions(self.size, false, self.scale)
        
        //let context = UIGraphicsGetCurrentContext() as CGContextRef
        //error: 'CGContext?' is not convertible to 'CGContextRef'
        //CGContextRef == CGContext, so it's saying "'CGContext?' is not convertible to 'CGContext'"
        //->you just need to unwrap it
        let context = UIGraphicsGetCurrentContext()!
        
        
        CGContextTranslateCTM(context, 0, self.size.height)
        CGContextScaleCTM(context, 1.0, -1.0);
        //CGContextSetBlendMode(context, kCGBlendModeNormal)
        //error: use of unresolved identifier 'kCGBlendModeNormal'
        //->See the CGBlendMode constants in the CGContext Reference, it's got enum.
        CGContextSetBlendMode(context, CGBlendMode.Normal)
        
        let rect = CGRectMake(0, 0, self.size.width, self.size.height) as CGRect
        CGContextClipToMask(context, rect, self.CGImage)
        tintColor.setFill()
        CGContextFillRect(context, rect)
        
        let newImage = UIGraphicsGetImageFromCurrentImageContext() as UIImage
        UIGraphicsEndImageContext()
        
        return newImage
    }
    
    var rounded: UIImage? {
        let imageView = UIImageView(image: self)
        imageView.layer.cornerRadius = min(size.height/2, size.width/2)
        imageView.layer.masksToBounds = true
        UIGraphicsBeginImageContextWithOptions(imageView.bounds.size, false, scale)
        guard let context = UIGraphicsGetCurrentContext() else { return nil }
        imageView.layer.renderInContext(context)
        let result = UIGraphicsGetImageFromCurrentImageContext()
        UIGraphicsEndImageContext()
        return result
    }
    var circle: UIImage? {
        let square = CGSize(width: min(size.width, size.height), height: min(size.width, size.height))
        let imageView = UIImageView(frame: CGRect(origin: CGPoint(x: 0, y: 0), size: square))
        imageView.contentMode = .ScaleAspectFill
        imageView.image = self
        imageView.layer.cornerRadius = square.width/2
        imageView.layer.masksToBounds = true
        UIGraphicsBeginImageContextWithOptions(imageView.bounds.size, false, scale)
        guard let context = UIGraphicsGetCurrentContext() else { return nil }
        imageView.layer.renderInContext(context)
        let result = UIGraphicsGetImageFromCurrentImageContext()
        UIGraphicsEndImageContext()
        return result
    }
}
