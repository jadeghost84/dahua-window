#include "YOLO.h"
#include "DHConnection.h"
#include <chrono>
#include <thread>
using namespace std;
Net_config yolo_nets[4] = {
	{0.5, 0.4,416, 416,"coco.names", "weight/yolov3/yolov3.cfg", "weight/yolov3/yolov3.weights", "weight/yolov3"},
	{0.5, 0.4, 608, 608,"coco.names", "weight/yolov4/yolov4.cfg", "weight/yolov4/yolov4.weights", "weight/yolov4"},
	{0.5, 0.4, 320, 320,"coco.names", "weight/yolo-fastest/yolo-fastest-xl.cfg", "weight/yolo-fastest/yolo-fastest-xl.weights", "weight/yolo-fastest"},
	{0.5, 0.4, 320, 320,"coco.names", "weight/yolobile/csdarknet53s-panet-spp.cfg", "weight/yolobile/yolobile.weights", "weight/yolobile"}
};
YOLO yolo_model1(yolo_nets[2]);
YOLO yolo_model2(yolo_nets[2]);
YOLO yolo_model3(yolo_nets[2]);
YOLO yolo_model4(yolo_nets[2]);
YOLO yolo_model5(yolo_nets[2]);
YOLO yolo_model6(yolo_nets[2]);
YOLO yolo_model7(yolo_nets[2]);
YOLO yolo_model8(yolo_nets[2]);
YOLO yolo_model9(yolo_nets[2]);
vector<YOLO> yolos;


void common(cv::Mat mat,int modelIndex) {
	//yolos[modelIndex].detect(mat);
	cv::imshow(to_string(modelIndex), mat);
	cv::waitKey(2);
	//cout << modelIndex << endl;
}
void test1(cv::Mat mat) {
	common(mat, 0);
}
void test2(cv::Mat mat) {
	common(mat, 1);
}
void test3(cv::Mat mat) {
	common(mat, 2);
}
void test4(cv::Mat mat) {
	common(mat, 3);
}
void test5(cv::Mat mat) {
	common(mat, 4);
}
void test6(cv::Mat mat) {
	common(mat, 5);
}
void test7(cv::Mat mat) {
	common(mat, 6);
}
void test8(cv::Mat mat) {
	common(mat, 7);
}
void test9(cv::Mat mat) {
	common(mat, 8);
}
Device d1 = { "设备1","192.168.2.102",37777, "admin", "csis0123",0 ,{"model1","model2","model3","model4"},{test1,test2 ,test3,test4,test5} };
Device d2 = { "设备2", "192.168.2.101",37777, "admin", "csis0123",0,{"model1","model2","model3"} ,{test6,test7,test8,test9 } };

/*
Device d3 = { "设备1","192.168.2.102",37777, "admin", "csis0123",0 ,{"model1","model2"},test3 };
Device d4 = { "设备2", "192.168.2.101",37777, "admin", "csis0123",0,{"model3"} ,test4 };
Device d5 = { "设备1","192.168.2.102",37777, "admin", "csis0123",0 ,{"model1","model2"},test5 };
Device d6 = { "设备2", "192.168.2.101",37777, "admin", "csis0123",0,{"model3"} ,test6 };
Device d7 = { "设备1","192.168.2.102",37777, "admin", "csis0123",0 ,{"model1","model2"},test7 };
Device d8 = { "设备2", "192.168.2.101",37777, "admin", "csis0123",0,{"model3"} ,test8 };
Device d9 = { "设备1","192.168.2.102",37777, "admin", "csis0123",0 ,{"model1","model2"},test9};*/
vector<Device> devices = { d1,d2};
int main() {
	//初始化推理模型
	for (int i = 0; i < 9; i++) {
		YOLO yolo_model(yolo_nets[2]);
		yolos.push_back(yolo_model);
	}
	DHConnection dHconnect = DHConnection();
	bool initSDKResult = dHconnect.initSDK();
	//添加设备
	vector<Device>::iterator it = devices.begin();
	for (; it != devices.end(); ++it)
	{
		dHconnect.addDevice((*it));
	}
	dHconnect.startPlay();
	Sleep(1000000);
	dHconnect.closePlay();
	

	/*std::thread th1(aa, handleMat1, 1, ip1);
	std::thread th2(aa, handleMat2,2, ip2);
	std::thread th3(aa, handleMat3,3, ip1);
	std::thread th4(aa, handleMat4,4, ip2);
	std::thread th5(aa, handleMat5,5, ip1);
	std::thread th6(aa, handleMat6,6, ip2);
	std::thread th7(aa, handleMat7,7, ip1);
	std::thread th8(aa, handleMat8,8, ip2);
	std::thread th9(aa, handleMat9,9, ip1);
	th1.join();
	th2.join();
	th3.join();
	th4.join();
	th5.join();
	th6.join();
	th7.join();
	th8.join();
	th9.join();*/

	//dHconnect2.closeConnect();
	return 0;
}