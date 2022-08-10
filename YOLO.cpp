#include "YOLO.h"

YOLO::YOLO(Net_config config)
{
	std::cout << "Net use " << config.netname << std::endl;
	this->confThreshold = config.confThreshold;
	this->nmsThreshold = config.nmsThreshold;
	this->inpWidth = config.inpWidth;
	this->inpHeight = config.inpHeight;
	strcpy_s(this->netname, config.netname.c_str());
	
	std::ifstream ifs(config.classesFile.c_str());
	std::string line;
	while (std::getline(ifs, line)) this->classes.push_back(line);

	this->net = cv::dnn::readNetFromDarknet(config.modelConfiguration, config.modelWeights);
	this->net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
	this->net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
}

void YOLO::detect(cv::Mat& frame)
{
	cv::Mat blob;
	cv::dnn::blobFromImage(frame, blob, 1 / 255.0, cv::Size(this->inpWidth, this->inpHeight), cv::Scalar(0, 0, 0), true, false);
	this->net.setInput(blob);
	std::vector<cv::Mat> outs;
	this->net.forward(outs, this->net.getUnconnectedOutLayersNames());
	this->postprocess(frame, outs);

	std::vector<double> layersTimes;
	double freq = cv::getTickFrequency() / 1000;
	double t = net.getPerfProfile(layersTimes) / freq;
	std::string label = cv::format("%s Inference time : %.2f ms", this->netname, t);
	putText(frame, label, cv::Point(0, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
}

void YOLO::postprocess(cv::Mat& frame, const std::vector<cv::Mat>& outs)
{
	std::vector<int> classIds;
	std::vector<float> confidences;
	std::vector<cv::Rect> boxes;

	for (size_t i = 0; i < outs.size(); ++i)
	{
		// Scan through all the bounding boxes output from the network and keep only the
		// ones with high confidence scores. Assign the box's class label as the class
		// with the highest score for the box.
		float* data = (float*)outs[i].data;
		for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
		{
			cv::Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
			cv::Point classIdPoint;
			double confidence;
			// Get the value and location of the maximum score
			minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
			if (confidence > this->confThreshold)
			{
				int centerX = (int)(data[0] * frame.cols);
				int centerY = (int)(data[1] * frame.rows);
				int width = (int)(data[2] * frame.cols);
				int height = (int)(data[3] * frame.rows);
				int left = centerX - width / 2;
				int top = centerY - height / 2;

				classIds.push_back(classIdPoint.x);
				confidences.push_back((float)confidence);
				boxes.push_back(cv::Rect(left, top, width, height));
			}
		}
	}

	// Perform non maximum suppression to eliminate redundant overlapping boxes with
	// lower confidences
	std::vector<int> indices;
	cv::dnn::NMSBoxes(boxes, confidences, this->confThreshold, this->nmsThreshold, indices);
	for (size_t i = 0; i < indices.size(); ++i)
	{
		int idx = indices[i];
		cv::Rect box = boxes[idx];
		this->drawPred(classIds[idx], confidences[idx], box.x, box.y,
			box.x + box.width, box.y + box.height, frame);
	}
}
void YOLO::drawPred(int classId, float conf, int left, int top, int right, int bottom, cv::Mat& frame)
{
	//Draw a rectangle displaying the bounding box
	rectangle(frame, cv::Point(left, top), cv::Point(right, bottom), cv::Scalar(0, 0, 255), 3);

	//Get the label for the class name and its confidence
	std::string label = cv::format("%.2f", conf);
	if (!this->classes.empty())
	{
		CV_Assert(classId < (int)this->classes.size());
		label = this->classes[classId] + ":" + label;

	}

	//Display the label at the top of the bounding box
	int baseLine;
	cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

	top = cv::max(top, labelSize.height);
	//rectangle(frame, Point(left, top - int(1.5 * labelSize.height)), Point(left + int(1.5 * labelSize.width), top + baseLine), Scalar(0, 255, 0), FILLED);

	cv::putText(frame, label, cv::Point(left, top), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0, 255, 0), 1);
}
