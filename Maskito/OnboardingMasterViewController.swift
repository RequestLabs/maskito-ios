import UIKit

class OnboardingMasterViewController: UIViewController {
    
    @IBOutlet var pageControl: UIPageControl?
    
    @IBAction func button(sender: AnyObject) {
        User.completeOnboarding()
        Nav.handle()
    }
    
    
    @IBOutlet weak var skipBtn: UIButton!
    
    @IBAction func nextBtn(sender: AnyObject) {
        self.navigateToPage(currentPage+1)
        
        print("nextBtn")
        //
        print(currentPage+1)
        //
        print(numberOfPages)
        
        //
        if (currentPage+1 == numberOfPages) {
            User.completeOnboarding()
            Nav.handle()
        }
    }
    
    

    
    private let scrollView: UIScrollView!
    private var controllers: [UIViewController]!
    private var scrollviewViewConstraint: NSArray?
    
    var currentPage: Int {
        get {
            let page = Int((scrollView.contentOffset.x / view.bounds.size.width))
            return page
        }
    }
    
    var numberOfPages: Int {
        get {
            return self.controllers.count
        }
    }
    
    required init?(coder aDecoder: NSCoder) {
        scrollView = UIScrollView()
        scrollView.showsHorizontalScrollIndicator = false
        scrollView.showsVerticalScrollIndicator = false
        scrollView.pagingEnabled = true
        
        controllers = Array()
        
        super.init(coder: aDecoder)
    }
    
    override init(nibName nibNameOrNil: String?, bundle nibBundleOrNil: NSBundle?) {
        scrollView = UIScrollView()
        controllers = Array()
        
        super.init(nibName: nibNameOrNil, bundle: nibBundleOrNil)
    }
    
    override func viewDidLoad() {
        
        super.viewDidLoad()
        
        pageControl?.addTarget(self, action: Selector(pageControllDidTouch()), forControlEvents: .TouchUpInside)
        
        scrollView.delegate = self
        scrollView.translatesAutoresizingMaskIntoConstraints = false
        view.insertSubview(scrollView, atIndex: 0)
        
        view.addConstraints(NSLayoutConstraint.constraintsWithVisualFormat("V:|-0-[scrollview]-0-|", options:[], metrics: nil, views: ["scrollview":scrollView]))
        view.addConstraints(NSLayoutConstraint.constraintsWithVisualFormat("H:|-0-[scrollview]-0-|", options:[], metrics: nil, views: ["scrollview":scrollView]))
        
    }
    
    override func viewWillAppear(animated: Bool) {
        super.viewWillAppear(animated)
        
        pageControl?.numberOfPages = self.numberOfPages
        pageControl?.currentPage = 0
        
    }
    
}

// MARK: - UIScrollViewDelegate
extension OnboardingMasterViewController : UIScrollViewDelegate {
    
    func scrollViewDidEndDecelerating(scrollView: UIScrollView) {
        pageControl?.currentPage = currentPage
    }
    
    func scrollViewDidEndScrollingAnimation(scrollView: UIScrollView) {
        pageControl?.currentPage = currentPage
    }
    
}

// MARK: - Helper Methods
extension OnboardingMasterViewController {
    
    func pageControllDidTouch() {
        if let pageControl = pageControl {
            navigateToPage(pageControl.currentPage)
        }
    }
    
    func addViewController(vc: UIViewController) -> Void {
        controllers.append(vc)
        
        vc.view.translatesAutoresizingMaskIntoConstraints = false
        scrollView.addSubview(vc.view)
        
        let metrics = ["w":vc.view.bounds.size.width,"h":vc.view.bounds.size.height]
        vc.view.addConstraints(NSLayoutConstraint.constraintsWithVisualFormat("V:[view(h)]", options:[], metrics: metrics, views: ["view":vc.view]))
        vc.view.addConstraints(NSLayoutConstraint.constraintsWithVisualFormat("H:[view(w)]", options:[], metrics: metrics, views: ["view":vc.view]))
        scrollView.addConstraints(NSLayoutConstraint.constraintsWithVisualFormat("V:|-0-[view]|", options:[], metrics: nil, views: ["view":vc.view,]))
        
        if self.numberOfPages == 1 {
            scrollView.addConstraints(NSLayoutConstraint.constraintsWithVisualFormat("H:|-0-[view]", options:[], metrics: nil, views: ["view":vc.view,]))
        } else {
            let previousVC = controllers[self.numberOfPages - 2]
            let previousView = previousVC.view;
            
            scrollView.addConstraints(NSLayoutConstraint.constraintsWithVisualFormat("H:[previousView]-0-[view]", options:[], metrics: nil, views: ["previousView":previousView,"view":vc.view]))
            
            if let constraint = scrollviewViewConstraint{
                scrollView.removeConstraints(constraint as! [NSLayoutConstraint])
            }
            scrollviewViewConstraint = NSLayoutConstraint.constraintsWithVisualFormat("H:[view]-0-|", options:[], metrics: nil, views: ["view":vc.view])
            scrollView.addConstraints(scrollviewViewConstraint! as! [NSLayoutConstraint])
        }
    }
    
    private func navigateToPage(page: Int) {
        
        //
        Analytics.view("ONBOARDING PAGE \(page)")
        // Hide or Show - SKIP Button
        if (page+1 == self.numberOfPages) {
            self.skipBtn.hidden = true
        } else {
            self.skipBtn.hidden = false
        }
        
        if page < self.numberOfPages {
            var frame = scrollView.frame
            frame.origin.x = CGFloat(page) * frame.size.width
            scrollView.scrollRectToVisible(frame, animated: true)
        }
    }
    
}