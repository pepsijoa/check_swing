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

					line(img, Point(x, y), Point(cvRound(x + flowatxy.x), cvRound(y + flowatxy.y)), Scalar(0, 255, 0));
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


vector<cv::Mat> detect_max_opticalflow(string& videopath)
{
	vector<cv::Mat> maybeBat;

	VideoCapture capture(videopath);
	if (!capture.isOpened()) {
		std::cerr << "can't open video" << endl;
		return maybeBat;
	}
	int x, y, key;
	Mat flow, frame, prevFrame, img;
	
	vector<vector<double>> gradflow(Height, vector<double>(Width, 0));
	vector<vector<double>> pregradflow(Height, vector<double>(Width, 0));
	vector<vector<double>> prepregradflow(Height, vector<double>(Width, 0));


	
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
					if (gradflow[y][x] > max * th and max > 5.0f) {
						for (int winy = -winSize; winy <= winSize; winy++) {
							for (int winx = -winSize; winx <= winSize; winx++) {
								if (y + winy < 0 or y + winy >= Height * 0.75 or x + winx < 0 or x + winx >= Width) continue;
								if (pregradflow[y + winy][x + winx] > max * th or prepregradflow[y + winy][x + winx] > max * th) {
									Point candi(x, y);
									int roiwidth = winSize; int roiheight = winSize;
									Rect roi(candi.x - roiwidth, candi.y - roiheight, roiwidth, roiheight);
									roi = roi & Rect(0, 0, Width, Height);
									maybeBat.push_back(frame(roi));
									circle(img, Point(x, y), 3, Scalar(0, 0, 255), -1);
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
		imshow("res", img);
		key = waitKey(20);
		if (key >= 0) break;
	}
	return maybeBat;
}

//Mat SLICsegmentation(Mat image, int k)
//{
//
//	SLIC slic;
//	int x, y;
//	int height, width;
//	int numlabels; // Generated number of superpixels
//	int m_spcount = k; // Desired number of superpixels
//	double m_compactness = 20;// 20.0; // compactness factor (1-40)
//	height = image.rows;
//	width = image.cols;
//	unsigned int* ubuff = (unsigned int*)calloc(height * width, sizeof(unsigned int));
//	int* labels = (int*)calloc(height * width, sizeof(int));
//	
//	for (y = 0; y < height; y++) {
//		for (x = 0; x < width; x++) {
//			ubuff[y * width + x] = (int)image.at<Vec3b>(y, x)[0] + ((int)image.at<Vec3b>(y, x)[1] << 8) +
//				((int)image.at<Vec3b>(y, x)[2] << 16);
//		}
//	}
//
//	
//
//	slic.DoSuperpixelSegmentation_ForGivenNumberOfSuperpixels(ubuff, width, height, labels,
//		numlabels, m_spcount, m_compactness);
//
//	Mat result(height, width, CV_8UC3);
//	slic.DrawContoursAroundSegments(ubuff, labels, width, height, 0);
//	for (y = 0; y < height; y++) {
//		for (x = 0; x < width; x++) {
//			result.at<Vec3b>(y, x)[0] = ubuff[y * width + x] & 0xff;
//			result.at<Vec3b>(y, x)[1] = ubuff[y * width + x] >> 8 & 0xff;
//			result.at<Vec3b>(y, x)[2] = ubuff[y * width + x] >> 16 & 0xff;
//		}
//	}
//	
//	free(ubuff);
//	free(labels);
//	return result;
//	
//}


