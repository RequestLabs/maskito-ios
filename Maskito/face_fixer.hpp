//
//  face_fixer.hpp
//

#ifndef face_fixer2_face_fixer_hpp
#define face_fixer2_face_fixer_hpp

#include <array>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/image_io.h>

/* Constants */
double MAX_PIXELS_IN_ANY_DIMENSION = 3096;
double BLUR_PATCH_SIZE_RGB = 3;
double BLUR_PATCH_SIZE_HSV = 3;
double TEETH_SATURATION_FACTOR = 0.8;
double EYE_ENLARGEMENT_FACTOR = 1.07;
double EYE_SATURATION_FACTOR = 0.6;
double NOSE_SIDE_DISTANCE_THRESH = 0.15; // # less than
double NOSE_DOWN_DISTANCE_THRESH = 0.3;  //# greater than
std::vector<double> OPERATIONS_CHECKLIST = {1, 1, 1, 1, 1};

class face_fixer{
private:
    std::string shape_predictor_file_location = "FILE_PATH";
    std::string file_location;
    dlib::shape_predictor predictor;
    dlib::frontal_face_detector detector;
    cv::Mat image;
    cv::Mat image_hsv;
    std::vector<dlib::rectangle> bounding_boxes;
    long num_faces;
    std::vector<std::array<std::array<long, 2>, 68>> landmarks;
    std::vector<std::array<std::array<long, 2>, 68>> landmarks_new;
    
public:
    face_fixer();
    int fix_faces(std::string file_location_string);
    void find_faces();
    void improve_skin();
    void improve_smile();
    void improve_eyes();
    void improve_face_structure();
};

face_fixer::face_fixer(){
    detector = dlib::get_frontal_face_detector();
    dlib::deserialize(shape_predictor_file_location) >> predictor;
};

int face_fixer::fix_faces(std::string file_location_string){
    file_location = file_location_string;
    try {
        face_fixer::find_faces();
        if (OPERATIONS_CHECKLIST[0])
            face_fixer::improve_skin();
        if (OPERATIONS_CHECKLIST[1])
            face_fixer::improve_smile();
        if (OPERATIONS_CHECKLIST[2])
            face_fixer::improve_eyes();
        if (OPERATIONS_CHECKLIST[3])
            face_fixer::improve_face_structure();
    }
    catch (std::exception& e)
    {
        std::cout << "\nexception thrown!" << std::endl;
        std::cout << e.what() << std::endl;
    }
    return 0;
};

void face_fixer::find_faces(){
    dlib::array2d<dlib::rgb_pixel> image_dlib;
    dlib::load_image(image_dlib, file_location);
    dlib::pyramid_up(image_dlib);
    
    cv::Mat image = cv::imread(file_location,1);
    cv::cvtColor(image, image_hsv, CV_RGB2HSV);
    
    std::vector<dlib::rectangle> bounding_boxes = detector(image_dlib);
    
    num_faces = bounding_boxes.size();
    
    landmarks.resize(num_faces);
    
    for (unsigned long j = 0; j < num_faces; ++j)
    {
        dlib::full_object_detection shape = predictor(image_dlib, bounding_boxes[j]);
        
        for (unsigned long k = 0; k < shape.num_parts(); ++k){
            landmarks[j][k][0] = shape.part(k).x();
            landmarks[j][k][1] = shape.part(k).y();
        }
    }
    landmarks_new = landmarks;
};

void face_fixer::improve_skin(){
    
};

void face_fixer::improve_smile(){
    
};

void face_fixer::improve_eyes(){
    
};

void face_fixer::improve_face_structure(){
    
};


#endif
