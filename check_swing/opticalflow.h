#ifndef opticalflow_H
#define opticalflow_H

#include <string>
#include<opencv2/opencv.hpp>
#include <vector>

using namespace std;
vector<int> detect_max_opticalflow(string& videopath);
int good_matching(std::string& videopath, vector<cv::Mat> candi);
int maskgrab(cv::Mat img, int direction);
cv::Mat whiten(cv::Mat img);
void drawpoint(int bodyX, vector<int>batX, std::string& videopath);
//cv::Mat SLICsegmentation(cv::Mat image, int k);

#endif