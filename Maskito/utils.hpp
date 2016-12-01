#ifndef utils_hpp
#define utils_hpp

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

double max_of_cvmat(const cv::Mat &mat){
    double max = 0.0;
    for(int i=0; i<mat.rows; i++){
        for(int j=0; j<mat.cols; j++){
            max = std::fmax(mat.at<uchar>(i,j), max);
        };
    };
    return max;
};


void elementwise_product_cvmat_scalar(cv::Mat &imagemat, const double scalar){
    for(int i=0; i<imagemat.rows; i++){
        for(int j=0; j<imagemat.cols; j++){
            imagemat.at<cv::Vec3b>(i,j)[0] = imagemat.at<cv::Vec3b>(i,j)[0] * scalar;
            imagemat.at<cv::Vec3b>(i,j)[1] = imagemat.at<cv::Vec3b>(i,j)[1] * scalar;
            imagemat.at<cv::Vec3b>(i,j)[2] = imagemat.at<cv::Vec3b>(i,j)[2] * scalar;
        };
    };
};

void elementwise_product_cvmat(cv::Mat &imagemat, const std::vector<std::vector<float>> &vector_b){
    for(int i=0; i<imagemat.rows; i++){
        for(int j=0; j<imagemat.cols; j++){
            imagemat.at<cv::Vec3b>(i,j)[0] = imagemat.at<cv::Vec3b>(i,j)[0] * vector_b[i][j];
            imagemat.at<cv::Vec3b>(i,j)[1] = imagemat.at<cv::Vec3b>(i,j)[1] * vector_b[i][j];
            imagemat.at<cv::Vec3b>(i,j)[2] = imagemat.at<cv::Vec3b>(i,j)[2] * vector_b[i][j];
        };
    };
};

void elementwise_sum_cvmat(cv::Mat &imagemat, cv::Mat &imagemat2){
    /*NB sums accumulate in the first cvmat passed in!*/
    for(int i=0; i<imagemat.rows; i++){
        for(int j=0; j<imagemat.cols; j++){
            imagemat.at<cv::Vec3b>(i,j)[0] = imagemat.at<cv::Vec3b>(i,j)[0] + imagemat2.at<cv::Vec3b>(i,j)[0];
            imagemat.at<cv::Vec3b>(i,j)[1] = imagemat.at<cv::Vec3b>(i,j)[1] + imagemat2.at<cv::Vec3b>(i,j)[2];
            imagemat.at<cv::Vec3b>(i,j)[2] = imagemat.at<cv::Vec3b>(i,j)[2] + imagemat2.at<cv::Vec3b>(i,j)[2];
        };
    };
};

std::vector<std::vector<float>> elementwise_product(const std::vector<std::vector<float>> &vector_a, const std::vector<std::vector<float>> &vector_b){
    //Lets hope those two vectors are the same size
    std::vector<std::vector<float>> product = vector_a;
    long num_rows = vector_a.size();
    long num_cols = vector_a[0].size();
    
    for (int i = 0; i < num_rows; i++){
        for (int j = 0; j < num_cols; j++){
            product[i][j] = vector_a[i][j] * vector_b[i][j];
        };
    };
    return product;
};

std::vector<std::vector<float>> copy_subset_vector(const std::vector<std::vector<float>> &base_vector, std::vector<int> &indices){
    std::vector<std::vector<float>> output_vector;
    for (int i = 0; i < indices.size(); i++){
        output_vector.push_back(base_vector[indices[i]]);
    };
    return output_vector;
};

std::vector<std::vector<float>> invert_mask(const std::vector<std::vector<float>> &mask){
    long num_rows = mask.size();
    long num_cols = mask[0].size();
    
    std::vector<std::vector<float>> inverted_mask;
    
    for (int i = 0; i < num_rows; i++){
        for (int j = 0; j < num_cols; j++){
            inverted_mask[i][j] = 1.0 - mask[i][j];
        };
    };
    
    return inverted_mask;
};

std::vector<std::vector<float>> convex_mask(const long &num_rows, const long &num_cols, const std::vector<std::vector<float>> &shape){
    /*Returns an array of the same dimensions as pix_grid, consiting
     of a mask of 1's and 0's. If a point in pix_grid lies in the convex
     hull of shape, the mask contains a 1, otherwise it contains a 0.
     */
    cv::Subdiv2D subdiv;
    for (int i = 0; i < shape.size(); i++){
        cv::Point2f fp((float)shape[i][0], (float)shape[i][1]);
        subdiv.insert(fp);
    };
    
    std::vector<std::vector<float>> mask;
    mask.resize(num_rows);
    int edge;
    int vertex;
    
    for (int i = 0; i < num_rows; i++){
        mask[i].resize(num_rows);
        for (int j = 0; j < num_cols; j++){
            cv::Point2f fp((float)i, (float)j);
            int tmp = subdiv.locate(fp, edge, vertex);
            if(tmp == cv::Subdiv2D::PTLOC_INSIDE)
            {
                mask[i][j] = 1.0;
            }
            else
            {
                mask[i][j] = 0.0;
            };
        };
    };
    return mask;
};

#endif