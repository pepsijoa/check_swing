#include "opencv2/opencv.hpp"
#include <iostream>
#include <opencv2/features2d.hpp>


#define Width 600
#define Height 400

using namespace cv;
using namespace std;
int good_matching(std::string& videopath, vector<cv::Mat> candi)
{
	cout << candi.size() << endl;

	/*for (int i = 0; i < candi.size(); i++) {
		cv::imshow(to_string(i), candi[i]);
		waitKey(0);
	}*/
	VideoCapture capture(videopath);
	if (!capture.isOpened()) {
		std::cerr << "can't open video" << endl;
		return -1;
	}
	

	Ptr<Feature2D> feature = ORB::create();
	vector<vector<KeyPoint>>keypoints(candi.size()); //plus one for video
	vector<KeyPoint> keypoints2;
	int key;
	Mat frame2, frame;

	


	vector<cv::Mat> src(candi.size());
	vector<cv::Mat> des(candi.size());
	
	while (1)
	{
		bool isexist = capture.read(frame2);
		if (!isexist) break;
		resize(frame2, frame, Size(Width, Height));
		
		for (int i = 0; i < candi.size(); i++) {
			src[i] = candi[i];
			feature->detectAndCompute(src[i], Mat(), keypoints[i], des[i]);
			if (des[i].empty()) {
				continue;
			}

		}


		Mat src2;
		cvtColor(frame, src2, COLOR_BGR2GRAY);

		Mat desc2;
		feature->detectAndCompute(src2, Mat(), keypoints2, desc2);

		vector<vector<DMatch>> matches(candi.size());
		Ptr<DescriptorMatcher> matcher = BFMatcher::create(NORM_HAMMING);
		
		for (int i = 0; i < candi.size(); i++) {
			matcher->match(des[i], desc2, matches[i]);
			
			if (matches[i].empty()) {
				continue;
			}
			std::sort(matches[i].begin(), matches[i].end(), [](const DMatch& a, const DMatch& b) { return a.distance < b.distance; });
			
			vector<DMatch> good_matches(matches[i].begin(), matches[i].begin() + min(3, (int)matches[i].size()));
			
			for (const auto& match : good_matches) {
				if (match.trainIdx < 0 || match.trainIdx >= keypoints2.size()) {
					continue;
				}

				Point2f pt2f = keypoints2[match.trainIdx].pt; // Video frame keypoint
				Point pt = Point(pt2f); // Convert Point2f to Point
				circle(frame, pt, 3, Scalar(0, 0, 255), -1); // Draw circle
			}
		
		}

		cv::imshow("res", frame);
		key = waitKey(20);
		if (key >= 0) break;

	}
}
