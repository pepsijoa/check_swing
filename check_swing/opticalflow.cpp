#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video/tracking.hpp>
#include <vector>
#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include "SLIC.h"

#define Width 600
#define Height 400

#define LEFT 1
#define RIGHT 2

#define NOBAT 2
#define NOVIDEO 3

using namespace cv;

using namespace std;



int show_opticalflow(string& videopath)
{
	VideoCapture capture(videopath);
	if (!capture.isOpened()) {
		std::cerr << "can't open video" << endl;
		return -1;
	}
	int x, y, key;
	Mat flow, frame, prevFrame, img;
	Mat gradimg = Mat::zeros(Height, Width, CV_8UC1);
	vector<vector<double>> gradflow(Height, vector<double>(Width, 0));
	int frame_count = 0;
	while (1) {
		bool isexist = capture.read(frame);
		if (!isexist) break;
		resize(frame, img, Size(Width, Height));
		cv::cvtColor(img, frame, CV_BGR2GRAY);
		if (prevFrame.empty() == false) {
			// calculate optical flow
			calcOpticalFlowFarneback(prevFrame, frame, flow, 0.4, 2, 12, 2, 7, 1.2, 0);
			// By y += 5, x += 5 you can specify the grid
			double min = 999999;
			double max = 0;
			for (y = 0; y < Height; y += 3) {
				for (x = 0; x < Width; x += 3) {
					const Point2f flowatxy = flow.at<Point2f>(y, x);
					double grad = sqrt((flowatxy.x * flowatxy.x) + (flowatxy.y * flowatxy.y));
					gradflow[y][x] = grad;

					if (min > gradflow[y][x]) min = gradflow[y][x];
					if (max < gradflow[y][x]) max = gradflow[y][x];

					cv::line(img, Point(x, y), Point(cvRound(x + flowatxy.x), cvRound(y + flowatxy.y)), Scalar(0, 255, 0));
					circle(img, Point(x, y), 1, Scalar(0, 0, 0), -1);
				}
			}

			// min, max reulgar for grad for visualization
			for (y = 0; y < Height; y++) {
				for (x = 0; x < Width; x++) {
					gradimg.at<uchar>(y, x) = saturate_cast<int>(gradflow[y][x] - min) / (max - min) * 255.0;
				}
			}



			frame.copyTo(prevFrame);
		}
		else frame.copyTo(prevFrame);
		Mat colorgradimg;
		cv::cvtColor(gradimg, colorgradimg, COLOR_GRAY2BGR);
		Mat combined;
		cv::hconcat(colorgradimg, img, combined);
		cv::imwrite("./output/"+to_string(frame_count)+".bmp", combined);
		frame_count++;
		key = waitKey(20);
		if (key >= 0) break;
	}
	return 0;
}

int make_frame_video()
{
	vector<Mat> frames;

	for (int i = 0; i < 22; ++i) {
		// 프레임 생성: 단순 색상 변화를 주는 예제
		Mat img = imread("./output/" + to_string(i) + ".bmp", CV_LOAD_IMAGE_COLOR);
		frames.push_back(img);
	}

	// 출력할 영상 설정
	String output_path = "./output/video.avi";
	double fps = 13.0;
	VideoWriter writer(output_path, VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, Size(Width * 2, Height));

	if (!writer.isOpened()) {
		cerr << "Error: Could not open the video writer!" << endl;
		return -1;
	}

	// 프레임을 영상으로 저장
	for (const Mat& frame : frames) {
		writer.write(frame);
	}

	writer.release(); // 파일 쓰기 완료

	cout << "Video saved to " << output_path << endl;
	return 0;
}


int detect_max_opticalflow(string& videopath, Mat& output, int direction, int bodyX)
{
	
	int x, y, key;
	int checkswing = 0;
	Mat flow, frame, prevFrame, img;

	vector<cv::Mat> maybeBat;
	vector<int> batX;

	VideoCapture capture(videopath);

	if (!capture.isOpened()) {
		std::cerr << "can't open video" << endl;
		return NOVIDEO;
	}
	
	
	
	vector<vector<double>> gradflow(Height, vector<double>(Width, 0));
	vector<vector<double>> pregradflow(Height, vector<double>(Width, 0));
	vector<vector<double>> prepregradflow(Height, vector<double>(Width, 0));


	
	int frame_count = 0;
	int batXsize;
	int initial;
	
	if (direction == LEFT) initial = Width - 1;
	else initial = 0;
	batXsize = initial;
	
	
	while (1) {		
		bool isexist = capture.read(frame);
		if (!isexist) break;
		resize(frame, img, Size(Width, Height));

		

		cv::cvtColor(img, frame, CV_BGR2GRAY);
		if (prevFrame.empty() == false) {
			// calculate optical flow
			calcOpticalFlowFarneback(prevFrame, frame, flow, 0.4, 2, 12, 2, 7, 1.2, 0);
			// By y += 5, x += 5 you can specify the grid
			
			//bat line for red
			Point bodysp(bodyX, 0);
			Point bodyep(bodyX, img.rows - 1);
			cv::line(img, bodysp, bodyep, Scalar(0, 0, 255), 1);
			double max = 0.0f;
			float th = 0.9f;

			//cal flow
			for (y = Height*0.3; y < Height*0.75; y += 3) {
				for (x = 0; x < Width; x += 3) {

					const Point2f flowatxy = flow.at<Point2f>(y, x);
					double grad = sqrt((flowatxy.x * flowatxy.x) + (flowatxy.y * flowatxy.y));
					gradflow[y][x] = grad;
					if (max < grad) max = grad;

				}
			} 
			
			// get candidate
			int winSize = 60;
			for (y = Height * 0.3; y < Height * 0.75; y += 3) {
				for (x = 0; x < Width; x += 3) {
				
					int nx = x + flow.at<Point2f>(y, x).x;

					if (gradflow[y][x] > max * th and max > 5.0f) {


						for (int winy = -winSize; winy <= winSize; winy++) {
							for (int winx = -winSize; winx <= winSize; winx++) {
								if (y + winy < 0 or y + winy >= Height * 0.75 or x + winx < 0 or x + winx >= Width) continue;
								if (pregradflow[y + winy][x + winx] > max * th or prepregradflow[y + winy][x + winx] > max * th) {
									
									if (direction == LEFT) {
										if (nx < batXsize) {
											batXsize = nx;
											batX.push_back(batXsize);
											//bat line for green
											Point batsp(batXsize, 0);
											Point batep(batXsize, img.rows - 1);
											cv::line(img, batsp, batep, Scalar(0, 255, 0), 1);
											output = img.clone();
										}
									}

									else {
										if (nx > batXsize) {
											batXsize = nx;
											batX.push_back(batXsize);
											//bat line for green
											Point batsp(batXsize, 0);
											Point batep(batXsize, img.rows - 1);
											cv::line(img, batsp, batep, Scalar(0, 255, 0), 1);
											output = img.clone();

										}
											
									}
									

									// image crop for HOG
									/*Point candi(x, y);
									int roiwidth = winSize; int roiheight = winSize;
									Rect roi(candi.x - roiwidth, candi.y - roiheight, roiwidth, roiheight);
									roi = roi & Rect(0, 0, Width, Height);
									maybeBat.push_back(frame(roi));*/
									
									//circle(img, Point(x, y), 3, Scalar(0, 0, 255), -1);
									
								}
								
							}
						}
						

					}
				}
			}

			

			frame_count++;

			prepregradflow = pregradflow;
			pregradflow = gradflow;
			frame.copyTo(prevFrame);

			

		}
		else frame.copyTo(prevFrame);

		//imwrite("./output/res"+to_string(frame_count)+".bmp", img);
		cv::imshow("res", img);
		key = waitKey(20);
		if (key >= 0) break;
	}
	

	if (!batX.empty()) {
		if (direction == LEFT)
		{
			if (batX[batX.size() - 1] < bodyX) checkswing = 1;
			else checkswing = 0;
		}
		else {
			if (batX[batX.size() - 1] > bodyX) checkswing = 1;
			else checkswing = 0;
		}
	}

	else {
		return NOBAT;
	}

	return checkswing;
}


