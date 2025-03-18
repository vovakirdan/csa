#include "detect.h"
#include <opencv2/opencv.hpp>

bool detect_code(unsigned char* image, int width, int height) {
    cv::Mat img(height, width, CV_8UC4, image);
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_RGBA2GRAY);

    cv::Mat edges;
    cv::Canny(gray, edges, 50, 150);

    return cv::countNonZero(edges) > 100;
}
