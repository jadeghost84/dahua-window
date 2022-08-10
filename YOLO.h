#pragma once
#ifndef YOLO_MODEL
#define YOLO_MODEL


#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include <string>
#include <string.h>
#include <fstream>
#include<iostream>
struct Net_config
{
	float confThreshold; // Confidence threshold
	float nmsThreshold;  // Non-maximum suppression threshold
	int inpWidth;  // Width of network's input image
	int inpHeight; // Height of network's input image
	std::string classesFile;
	std::string modelConfiguration;
	std::string modelWeights;
	std::string netname;
};
class YOLO
{
public:
	YOLO(Net_config config);
	void detect(cv::Mat& frame);
private:
	float confThreshold;
	float nmsThreshold;
	int inpWidth;
	int inpHeight;
	char netname[20];
	std::vector<std::string> classes;
	cv::dnn::Net net;
	void postprocess(cv::Mat& frame, const std::vector<cv::Mat>& outs);
	void drawPred(int classId, float conf, int left, int top, int right, int bottom, cv::Mat& frame);
};

#endif // !YOLO_MODEL