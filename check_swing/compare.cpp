#include "opencv2/opencv.hpp"
#include <iostream>
using namespace cv;
using namespace std;
int good_matching()
{
	Mat src1 = imread("./img/test.jpg", IMREAD_GRAYSCALE);
	VideoCapture capture("./video/check1.mp4");
	if (!capture.isOpened()) {
		std::cerr << "can't open video" << endl;
		return -1;
	}
	imshow("res", src1);
	waitKey(0);
	//Mat src2 = imread("./img/player2.png", IMREAD_GRAYSCALE);

	int key;

	Ptr<Feature2D> feature = ORB::create();

	vector<KeyPoint> keypoints1, keypoints2;

	Mat desc1, frame;
	feature->detectAndCompute(src1, Mat(), keypoints1, desc1);
	Ptr<DescriptorMatcher> matcher = BFMatcher::create(NORM_HAMMING);

	while (1)
	{
		bool isexist = capture.read(frame);
		if (!isexist) break;

		Mat src2;
		cvtColor(frame, src2, COLOR_BGR2GRAY);

		Mat desc2;
		feature->detectAndCompute(src2, Mat(), keypoints2, desc2);

		vector<DMatch> matches;
		matcher->match(desc1, desc2, matches);

		Mat dst;

		std::sort(matches.begin(), matches.end());
		vector<DMatch> good_matches(matches.begin(), matches.end());


		cv::drawMatches(src1, keypoints1, src2, keypoints2, good_matches, dst,
			Scalar::all(-1), Scalar::all(-1), vector<char>(),
			DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

		cv::imshow("dst", dst);
		key = waitKey(20);
		if (key >= 0) break;

		keypoints2.clear();
	}

}