#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video/tracking.hpp>
#include <vector>
#include <stdio.h>
#include <Windows.h>
#include <iostream>
#define Width 600
#define Height 400
using namespace cv;

using namespace std;

string videopath = "./video/check2.mp4";
string imgpath = "./img/player1.png";
int main(void) {

	VideoCapture capture(videopath);
	if (!capture.isOpened()) {
		std::cerr << "can't open video" << endl;
		return -1;
	}
	int x, y, key;
	Mat flow, frame, prevFrame, img;
	vector<double> gradflow(Height * Width, 0);


	while (1) {
		bool isexist = capture.read(frame);
		if (!isexist) break;
		resize(frame, img, Size(Width, Height));
		cvtColor(img, frame, CV_BGR2GRAY);
		if (prevFrame.empty() == false) {
			// calculate optical flow
			calcOpticalFlowFarneback(prevFrame, frame, flow, 0.4, 2, 12, 2, 7, 1.2, 0);
			// By y += 5, x += 5 you can specify the grid
			for (y = 0; y < Height; y += 5) {
				for (x = 0; x < Width; x += 5) {
					const Point2f flowatxy = flow.at<Point2f>(y, x);

					line(img, Point(x, y), Point(cvRound(x + flowatxy.x), cvRound(y + flowatxy.y)), Scalar(0, 255, 0));
					circle(img, Point(x, y), 1, Scalar(0, 0, 0), -1);
				}
			}

			frame.copyTo(prevFrame);
		}
		else frame.copyTo(prevFrame);
		imshow("res", img);
		key = waitKey(20);
		if (key >= 0) break;
	}
	return 0;

}
