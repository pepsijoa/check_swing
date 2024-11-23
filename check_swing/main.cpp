#include<opencv2/opencv.hpp>
#include <iostream>
#include "opticalflow.h"
#include<vector>

using namespace cv;
using namespace std;


int main(void)
{
	string videopath = "./video/check4.mp4";
	vector<cv::Mat> candi = detect_max_opticalflow(videopath);
	
	return 0;
}

