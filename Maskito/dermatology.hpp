//
//  dermatology.hpp
//

#ifndef face_fixer2_dermatology_hpp
#define face_fixer2_dermatology_hpp

#include "utils.hpp"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "face_landmarks.hpp"

#include <iostream>

std::vector<std::vector<float>> find_skin_mask(const cv::Mat &image, const std::vector<std::array<std::array<long, 2>, 68>> &landmarks){
    /*find skin pixels in the face.  This is done by finding the
     convex hull defined by the face landmarks, then subtracting
     the convex hulls around the eyes and the mouth.*/
    long num_rows = image.dims[0];
    long num_cols = image.dims[1];
    
    std::vector<std::vector<long>> face = copy_subset_vector(landmarks, face_dlib);
    std::vector<std::vector<long>> left_eye = copy_subset_vector(landmarks, leye_dlib);
    std::vector<std::vector<long>> right_eye = copy_subset_vector(landmarks, reye_dlib);
    std::vector<std::vector<long>> teeth = copy_subset_vector(landmarks, innermouth_dlib);
    std::vector<std::vector<long>> left_eyebrow = copy_subset_vector(landmarks, leyebrow_dlib);
    std::vector<std::vector<long>> right_eyebrow = copy_subset_vector(landmarks, reyebrow_dlib);
    std::vector<std::vector<long>> _full_face = face.insert(face.end(), left_eyebrow.begin(), left_eyebrow.end());
    std::vector<std::vector<long>> full_face = _full_face.insert( _full_face.end(), right_eyebrow.begin(), right_eyebrow.end());
    
    std::vector<std::vector<float>> left_eye_mask = convex_mask(num_rows, num_cols, left_eye);
    std::vector<std::vector<float>> invert_left_eye_mask = invert_mask(left_eye_mask);
    std::vector<std::vector<float>> right_eye_mask = convex_mask(num_rows, num_cols, right_eye);
    std::vector<std::vector<float>> invert_right_eye_mask = invert_mask(right_eye_mask);
    std::vector<std::vector<float>> teeth_mask = convex_mask(num_rows, num_cols, teeth);
    std::vector<std::vector<float>> invert_teeth_mask = invert_mask(teeth_mask);
    std::vector<std::vector<float>> face_mask = convex_mask(num_rows, num_cols, full_face);
    
    std::vector<std::vector<float>> skin_a = elementwise_product(face_mask, invert_left_eye_mask);
    std::vector<std::vector<float>> skin_b = elementwise_product(invert_right_eye_mask, invert_teeth_mask);
    std::vector<std::vector<float>> skin_mask = elementwise_product(skin_a, skin_b);
    
    return skin_mask;
};

std::vector<std::vector<float>> find_face_mask(const cv::Mat &image, const std::vector<std::array<std::array<long, 2>, 68>> &landmarks){
    /*find skin pixels in the face.  This is done by finding the
     convex hull defined by the face landmarks, then subtracting
     the convex hulls around the eyes and the mouth.*/
    long num_rows = image.dims[0];
    long num_cols = image.dims[1];
    
    std::vector<std::vector<long>> face = copy_subset_vector(landmarks, face_dlib);
    std::vector<std::vector<float>> face_mask = convex_mask(num_rows, num_cols, full_face);
    return face_mask;
};

double estimate_image_blur(const cv::Mat &image, const std::vector<std::array<std::array<long, 2>, 68>> &landmarks){
    long num_rows = image.dims[0];
    long num_cols = image.dims[1];
    long num_pixels = num_rows * num_cols;
    
    cv::Mat image_gray;
    cv::cvCvtColor(image, image_gray, CV_RGB2GRAY);
    cv::Mat image_gray_equalized = image_gray
    
    std::vector<std::vector<float>> mask_face_outine = find_face(image, landmarks);
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(double clipLimit = 0.01, cv::Size tileGridSize=cv::Size(8,8));
    
    clahe->apply(image_gray, image_gray_equalized);
    
    for(int i=0; i<image_gray_equalized.rows; i++){
        for(int j=0; j<image_gray_equalized.cols; j++){
            image_gray_equalized.at<uchar>(i,j) = image_gray_equalized.at<uchar>(i,j) * mask_face_outine[i][j];
        };
    };
    
    cv::GaussianBlur( image_gray_equalized, image_gray_equalized, Size(3,3), 0, 0, cv::BORDER_DEFAULT);
    
    double max_val = max_of_cvmat(image_gray_equalized);
    double blur_metric = 1.0 - (0.5 + std::exp(std::log(0.4) - 0.85 * max_val));
    
    return blur_metric;
};

void smooth_layers(cv::Mat &image, int sigma){
    /*Apply a gaussian_filter to each layer of the image,
     one layer at a time (not all three layers at once).*/
    //TODO: not sure if image and source can be the same
    cv::GaussianBlur(image, image, Size(sigma, sigma), 0, 0, cv::BORDER_DEFAULT);
};

void improve_skin(const cv::Mat &image, const cv::Mat &image_hsv, const vector<std::array<std::array<long, 2>, 68>> &landmarks, int blur_patch_size_hsv, int blur_patch_size_rgb){
    /*returns a new image in which the skin has been smoothed.*/
    double blur_factor = estimate_image_blur(image, landmarks);
    
    std::vector<std::vector<float>> skin_mask = find_skin_mask(image, landmarks);
    std::vector<std::vector<float>> not_skin_mask = invert_mask(skin_mask);
    
    /*HSV smoothing*/
    smooth_layers(image_hsv, blur_patch_size_hsv);
    cv::Mat image_hsv_copy = image_hsv;
    
    elementwise_product_cvmat(image_hsv, skin_mask);
    elementwise_product_cvmat(image_hsv_copy, not_skin_mask);
    elementwise_sum_cvmat(image_hsv, image_hsv_copy);
    cv::cvCvtColor(image_hsv, image, CV_HSV2RGB);
    cv::Mat image_original = image;
    
    /*RGB smoothing*/
    smooth_layers(image, blur_patch_size_rgb);
    cv::Mat image_rgb_copy = image_original;
    cv::Mat image_rgb_copy2 = image_original;
    elementwise_product_cvmat(image, skin_mask);
    elementwise_product_cvmat(image_rgb_copy, skin_mask);
    elementwise_product_cvmat(image_rgb_copy2, not_skin_mask);
    
    elementwise_product_cvmat_scalar(image, blur_factor);
    elementwise_product_cvmat_scalar(image_rgb_copy, 1.0 - blur_factor);
    
    elementwise_sum_cvmat(image, image_rgb_copy2);
    elementwise_sum_cvmat(image, image_rgb_copy);
};
#endif
