
#import <Foundation/Foundation.h>
#import "find_face.h"
#import "PHItypes.h"
#include <mutex>

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/image_io.h>
#include <dlib/opencv.h>

#include <opencv2/opencv.hpp>

#include <dlib/matrix.h>
#include <dlib/rand.h>
#include <dlib/timing.h>

#include "fft_stuff.hpp"

using namespace std;


struct tracker_rect {
    dlib::correlation_tracker tracker;
    dlib::rectangle lastone;
};

cv::Mat makeMat(CVPixelBufferRef buffer) {
    CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
    //size_t width = CVPixelBufferGetWidth(buffer);
    //size_t height = CVPixelBufferGetHeight(buffer);
    size_t rowbytes = CVPixelBufferGetBytesPerRow(buffer);
    char * ptr = (char * )CVPixelBufferGetBaseAddress(buffer); // ASSUMES IMAGE IS RGBA CHAR!!!
    
    int h = (int)CVPixelBufferGetHeight(buffer);
    int w = (int)CVPixelBufferGetWidth(buffer);
    
    cv::Mat mat(h, w, CV_8UC4, ptr, rowbytes);
    return mat;
}

@implementation FaceFinder {
    dlib::shape_predictor predictor;
    dlib::frontal_face_detector detector;
    
    bool dlibDone;
    bool doPredict;
    
    NSMutableArray * facesAverage;
    NSUInteger movingAverageCount;
    int retrackAfter;
    int iter;
    
    std::mutex applmtx;
    std::mutex dlibmtx;
    
    std::vector<dlib::rectangle> dlibRects;
    NSArray * appleRects;
    
    dlib::rectangle face_loc;
    CIDetector * appleFace;
    dispatch_queue_t faceQueue;
    
}

-(FaceFinder *)init {
    self = [super init];
    if (self) {
        
        doPredict = false;
        dlibDone = true;
        
        iter = 0;
        retrackAfter = 3;

        NSArray *dirPaths;
        NSString *docsDir;
        dirPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,
                                                       NSUserDomainMask, YES);
        docsDir = [dirPaths objectAtIndex:0];
        
        
        
        //NSString * dat_file = [[NSBundle mainBundle] pathForResource:@"facemarks" ofType:@"dat"];
        detector = dlib::get_frontal_face_detector();
        
        NSString *dat_file =
        [docsDir stringByAppendingPathComponent:@"facemarks.dat"];
        
        
        facesAverage = [[NSMutableArray alloc] init];
        faceQueue = dispatch_queue_create("com.PHI.faceQueue", DISPATCH_QUEUE_CONCURRENT);
        dispatch_async(faceQueue, ^{
            dlib::deserialize(dat_file.UTF8String) >> predictor;
            doPredict = true;
        });
        movingAverageCount = 0;
        
    }
    
    return self;
};

-(FaceFinder *) initWithRetrack: (int) _retrackAfter {
    self = [self init];
    if (self) {
        retrackAfter = _retrackAfter;
    }
    return self;
}


-(void)retrackInImage:(const cv::Mat &) smallMat {
    // Asynchronously find the faces using dlib's face detector
    cv::Mat smallMatCopy = smallMat;
    dispatch_async(faceQueue, ^{
        dlib::cv_image<dlib::rgb_pixel> smallImgCopy(smallMatCopy);
        std::vector<dlib::rectangle> faces = detector(smallImgCopy);
        
        // Update rects inside mutex
        dlibmtx.lock();
        dlibRects = faces;
        dlibmtx.unlock();
        
        dlibDone = true;
    });
}

-(std::vector<dlib::rectangle>) getRectsWithScale:(int) scale {
    std::vector<dlib::rectangle> localRects;
    dlibmtx.lock();
    for (auto smallRect : dlibRects) {
        dlib::rectangle faceRect = dlib::rectangle(
                                                   static_cast<long>(smallRect.left() * scale),
                                                   static_cast<long>(smallRect.top() * scale),
                                                   static_cast<long>(smallRect.right() * scale),
                                                   static_cast<long>(smallRect.bottom() * scale)
                                                   );
        localRects.push_back(faceRect);
    }
    dlibmtx.unlock();
    return localRects;
}

-(NSArray *) facesPointsInBigImage:(CVPixelBufferRef)bigBuff andSmallImage: (CVPixelBufferRef)smallBuff withScale: (int) scale {
    //Convert CamImages into dlib images. Need to unlock the buffers once done.
    cv::Mat bigMat = makeMat(bigBuff);
    dlib::cv_image<dlib::rgb_alpha_pixel> bigImg(bigMat);
    
    cv::Mat smallMatWithA = makeMat(smallBuff);
    cv::Mat smallMat;
    cv::cvtColor(smallMatWithA, smallMat, CV_BGRA2RGB);
    dlib::cv_image<dlib::rgb_pixel> smallImg(smallMat);
    
    if (dlibDone) {
        dlibDone = false;
        [self retrackInImage:smallMat];
    }
    
    // Resize rectangles and get a copy
    std::vector<dlib::rectangle> rects = [self getRectsWithScale: scale];
    
    //Unlock the buffers
    CVPixelBufferUnlockBaseAddress(bigBuff, kCVPixelBufferLock_ReadOnly);
    CVPixelBufferUnlockBaseAddress(smallBuff, kCVPixelBufferLock_ReadOnly);
    
    
    // Got face points outside mutex
    NSMutableArray * arr = [[NSMutableArray alloc] init];
    if (!doPredict) {
        return arr;
    }
    for (auto faceRect : rects) {
        NSMutableArray * internalArr = [[NSMutableArray alloc] init];
        dlib::full_object_detection res = predictor(bigImg, faceRect);
        for (int pidx = 0; pidx < res.num_parts(); ++pidx) {
            [internalArr addObject: [NSValue valueWithPhiPoint:PhiPoint{static_cast<int>(res.part(pidx).x()), static_cast<int>(res.part(pidx).y())}]];
        }
        [arr addObject: internalArr];
    }
    return arr;
}

CGPoint CGPointAdd(CGPoint p1, CGPoint p2)
{
    return CGPointMake(p1.x + p2.x, p1.y + p2.y);
}

CGPoint CGPointAdjustScaling(CGPoint p1, double v1)
{
    return CGPointMake(p1.x * v1, p1.y * v1);
}

@end
