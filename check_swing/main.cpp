#include<opencv2/opencv.hpp>
#include <iostream>
#include "opticalflow.h"
#include<vector>

using namespace cv;
using namespace std;

#define LEFT 1
#define RIGHT 0

Mat get_frame(float num, string&videopath)
{
    VideoCapture cap(videopath);
    if (!cap.isOpened()) {
        cerr << "Error: Cannot open video file!" << endl;
    }

    int totalFrames = static_cast<int>(cap.get(CAP_PROP_FRAME_COUNT));
    int targetFrame = totalFrames * num;
    
    cap.set(CAP_PROP_POS_FRAMES, targetFrame); // 프레임 위치 설정

    Mat frame;
    if (cap.read(frame)) {
        imshow("frame", frame);
        waitKey(0);
        cap.release();
        return frame;
    }
}

int main(void)
{
	string videopath = "./video/check3.mp4";
    Mat img = get_frame(0.3, videopath); 
	int body_x = maskgrab(img, LEFT);

	//vector<int> candi = detect_max_opticalflow(videopath);
	
	//drawpoint(body_x, candi, videopath);

	return 0;
}

