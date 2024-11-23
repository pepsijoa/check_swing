#ifndef opticalflow_H
#define opticalflow_H

#include <string>
#include<opencv2/opencv.hpp>
#include <vector>

using namespace std;
vector<cv::Mat> detect_max_opticalflow(string& videopath);

//cv::Mat SLICsegmentation(cv::Mat image, int k);

#endif