#include <opencv2/core/core.hpp>
#include<opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
 
using namespace cv;
using namespace std;

#define LEFT 1
#define RIGHT 0

#define Width 600
#define Height 400

cv::Mat whiten(cv::Mat img)
{
	Mat img2 = img.clone();

	float ratio = 0.5;
	Rect under(0, img.rows * ratio, img.cols, img.rows * ratio);
	Mat cropped = img(under);

	Mat gray;
	cvtColor(cropped, gray, COLOR_BGR2GRAY);
	
	Mat th;
	threshold(gray, th, 240, 255, THRESH_BINARY);
	

	morphologyEx(th, gray, MORPH_OPEN, Mat(), Point(-1, -1), 3);
	
	for (int y = 0; y < img.rows; y++) {
		for (int x = 0; x < img.cols; x++) {
			if (y <= img.rows * ratio) {
				img2.at<Vec3b>(y, x)[0] = 0;
				img2.at<Vec3b>(y, x)[1] = 0;
				img2.at<Vec3b>(y, x)[2] = 0;
			}
			else {
				img2.at<Vec3b>(y, x)[0] = th.at<uchar>(y- img.rows * ratio,x);
				img2.at<Vec3b>(y, x)[1] = th.at<uchar>(y- img.rows * ratio, x);
				img2.at<Vec3b>(y, x)[2] = th.at<uchar>(y- img.rows * ratio, x);
			}
		}
	}
	return img2;
}


int maskgrab(cv::Mat img, int direction)
{
	Mat img2;
	resize(img, img2, Size(Width, Height));
	int x, y;

	//Rect rectangle(0, 0, width - 1, height - 1);
	Rect rectangle(0, 0, 0, 0);
	Mat mask = Mat::ones(Height, Width, CV_8UC1) * GC_PR_BGD;

	Mat input_mask2 = whiten(img2);
	Mat input_mask;
	resize(input_mask2, input_mask, Size(Width, Height));

	
	for (y = 0; y < Height; y++) {
		for (x = 0; x < Width; x++) {
			if (input_mask.at<Vec3b>(y, x)[0] == 0 and
				input_mask.at<Vec3b>(y, x)[1] == 0 and
				input_mask.at<Vec3b>(y, x)[2] == 0)
				mask.at<uchar>(y, x) = GC_BGD;

			if (input_mask.at<Vec3b>(y, x)[0] == 255 and
				input_mask.at<Vec3b>(y, x)[1] == 255 and
				input_mask.at<Vec3b>(y, x)[2] == 255)
				mask.at<uchar>(y, x) = GC_FGD;
		}
	}

	Mat mask_result = Mat::zeros(Height, Width, CV_8UC1);
	Mat bg, fg;

	grabCut(img2, mask, rectangle, bg, fg, 8, GC_INIT_WITH_MASK);

	for (y = 0; y < Height; y++) {
		for (x = 0; x < Width; x++) {
			if (mask.at<uchar>(y, x) == GC_FGD || mask.at<uchar>(y, x) == GC_PR_FGD) {
				mask_result.at<uchar>(y, x) = 255;

			}
		}
	}
	Mat result;
    medianBlur(mask_result, result, 3);
	
	int maxbody;
	if (direction == LEFT) {
		maxbody = img2.cols;
	}
	else {
		maxbody = 0;
	}

	for (y = 0; y < Height; y++) {
		for (x = 0; x < Width; x++) {
			if (mask_result.at<uchar>(y,x) == 255) {
				if (direction == LEFT) {
					if (maxbody > x) maxbody = x;
				}
				else {
					if (maxbody < x) maxbody = x;
				}
			}
		}
	}
	
	imshow("mask_result", mask_result);
	waitKey(0);

	return maxbody;

}