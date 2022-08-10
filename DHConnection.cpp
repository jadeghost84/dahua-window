#include "DHConnection.h"




void DisConnectFunc(LLONG lLoginID, char* pchDVRIP, LONG nDVRPort, LDWORD dwUser) {
	std::cout << "设备断线" << std::endl;
	return;
}
//设备自动重连回调函数
void AutoReConnectFunc(LLONG lLoginID, char* pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	std::cout << "自动重连成功." << std::endl;
	return;
}


//实时数据回调函数，在此处理原始数据
void RealDataCallBackEx(LLONG lRealHandle, DWORD dwDataType, BYTE* pBuffer, DWORD dwBufSize, LLONG param, LDWORD playPorts)
{
	//cout << playPort << endl;
	BOOL bInput = FALSE;
	if (0 != lRealHandle)
	{
		switch (dwDataType) {
		case 0:
			//原始音视频混合数据
			//printf("receive real data, param: pBuffer[%p]\n", pBuffer);
			//cout << dwBufSize;
			//DHConnection thisClass = *(DHConnection*)pthis;
		{
			vector<LONG> _playPorts = *(vector<LONG>*)playPorts;
			std::vector<LONG>::iterator playPort = _playPorts.begin();
			for (; playPort != _playPorts.end(); ++playPort) {
				bInput = PLAY_InputData((*playPort), pBuffer, dwBufSize);
				if (!bInput){
					printf("input data error: %d\n", PLAY_GetLastError((*playPort)));
				}
			}

		}
			break;
		case 1:
			//标准视频数据

			break;
		case 2:{
			
			break;
		}
		case 3:
			//pcm 音频数据

			break;
		case 4:
			//原始音频数据
			break;
		default:
			break;
		}
	}
}


void CALLBACK DecCBFun(LONG nPort, char* pBuf, LONG nSize, FRAME_INFO* pFrameInfo, void* _this, LONG nReserved2)
{
	//移走数据后，快速返回,即开始解码回调下一帧数据;不要在回调中运行长事务，否则阻塞解码下一帧数据
	// pbuf里的数据是YUV I420格式的数据 
	if (pFrameInfo->nType == 3) //视频数据
	{
		//获取传过来的类对象指针
		DHConnection* __this = (DHConnection*)_this;
		//查看可操作标志，允许则执行转格式和回调
		if ((*__this).playPort2allowHandle[nPort]) {
			//将yuv格式转为cv::Mat格式
			cv::Mat mgMat;
			cv::Mat myuv(pFrameInfo->nHeight + pFrameInfo->nHeight / 2, pFrameInfo->nWidth, CV_8UC1, (unsigned char *)pBuf);
			cv::cvtColor(myuv, mgMat, CV_YUV420p2RGB);
			//异步执行回调函数
			std::future<void> handleFun = std::async(std::launch::async, (*__this).playPort2callBackFun[nPort], mgMat);
			(*__this).playPort2allowHandle[nPort] = false;
		}
		else {
			(*__this).playPort2allowHandle[nPort] = true;
		}
		//cv::Mat myuv(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC1, (unsigned char*)pBuf);
		
		//cvtColor(myuv, mgMat, CV_YUV2RGB_I420);
		//IplImage* mgIplImage = YUV420_To_IplImage_Opencv(pBuf, pFrameInfo->nWidth, pFrameInfo->nHeight);
		//mgMat = cv::cvarrToMat(mgIplImage);
		//std::cout << "( " << myuv.rows << "," << myuv.cols << "," << myuv.channels() << ")" << endl;
		//std::cout << "( " << mgMat.rows << "," << mgMat.cols << "," << mgMat.channels() << ")" << endl;
		
		
		//(* (DHConnection*)_this).playPort2callBackFun[nPort](mgMat);
		//cvReleaseImage(&mgIplImage);
		//printf("receive real data, param: pBuffer[%p]\n", pBuf);
	}
	else if (pFrameInfo->nType == T_AUDIO16)
	{
		//cout<<"Audio CallBack"<<endl;
	}
	else
	{
		printf("nType = %d\n", pFrameInfo->nType);
	}
	//cout << PLAY_GetSourceBufferRemain(PLAYPORT) << endl;
	return;
}

void ConvertLoginError2String(int nErrorCode, std::string &strErrorCode)
{
	switch (nErrorCode)
	{
	case 0:
		strErrorCode = "Login Success";
		break;

	case 1:
		strErrorCode = "Account or Password Incorrect";
		break;

	case 2:
		strErrorCode = "User Is Not Exist";
		break;

	case 3:
		strErrorCode = "Login Timeout";
		break;

	case 4:
		strErrorCode = "Repeat Login";
		break;

	case 5:
		strErrorCode = "User Account is Locked";
		break;

	case 6:
		strErrorCode = "User In Blacklist";
		break;

	case 7:
		strErrorCode = "Device Busy";
		break;

	case 8:
		strErrorCode = "Sub Connect Failed";
		break;

	case 9:
		strErrorCode = "Host Connect Failed";
		break;

	case 10:
		strErrorCode = "Max Connect";
		break;

	case 11:
		strErrorCode = "Support Protocol3 Only";
		break;

	case 12:
		strErrorCode = "UKey Info Error";
		break;

	case 13:
		strErrorCode = "No Authorized";
		break;

	case 18:
		strErrorCode = "Device Account isn't Initialized";
		break;

	default:
		strErrorCode = "Unknown Error";
		break;
	}

}


DHConnection::DHConnection()
{
	
}


bool DHConnection::initSDK()
{
	bool initFlag = CLIENT_Init(DisConnectFunc, 0);
	//LOG_SET_PRINT_INFO  stLogPrintInfo = { sizeof(stLogPrintInfo) };
	//CLIENT_LogOpen(&stLogPrintInfo);
	//CLIENT_LogClose();
	CLIENT_SetAutoReconnect(AutoReConnectFunc, 0);
	if (initFlag) {
		std::cout << "init SDK success" << std::endl;
		return true;
	}
	else {
		std::cout << "init SDK failed" << std::endl;
		return false;
	}
}


bool DHConnection::addDevice(Device device) {
	long long lLogin;
	NET_DEVICEINFO_Ex deviceInfo = { 0 };
	std::cout << " Login Device ..." << std::endl;
	int error = 0;
	lLogin = CLIENT_LoginEx2(device.ip, device.port, device.adminName, device.password, EM_LOGIN_SPEC_CAP_TCP, NULL, &deviceInfo, &error);//登陆设备
	if (lLogin != 0)
	{
		std::cout << device.name << ">>>Login Success" << std::endl;
		device.lLogin = lLogin;//登录成功，将登录句柄加入设备结构体
		devices.push_back(device);
		return true;
	}
	else
	{
		std::string errorStr;
		ConvertLoginError2String(error, errorStr);
		std::cout << device.name << ">>> Login Fail, error reason:" + errorStr << std::endl;
		return false;
	}
}


bool DHConnection::startPlay() {
	std::vector<Device>::iterator device = devices.begin();
	for (; device != devices.end(); ++device)
	{
		int nChannelID = 0;
		long long lRealPlay;
		lRealPlay = CLIENT_RealPlayEx((*device).lLogin, nChannelID, NULL, DH_RType_Realplay);
		if (lRealPlay != 0)
		{
			vector<LONG>* playPorts = new vector<LONG>;
			std::vector<matCallbackfun>::iterator callBackFun = (*device).callFuns.begin();
			for (; callBackFun != (*device).callFuns.end(); ++callBackFun)
			{
				LONG playPort = 0;
				// 获取空闲播放端口号
				if (PLAY_GetFreePort(&playPort)) {
					cout << (*device).name << ">>>Get playPort Success" << endl;
					//插入一条播放端口对应回调函数
					playPort2callBackFun.insert(pair<LONG, matCallbackfun>(playPort,(*callBackFun)));
					//插入一条播放端口对应可操作标志，表示是否执行回调函数，目前用于控制自控丢帧
					playPort2allowHandle.insert(pair<LONG, bool>(playPort, true));
					(*playPorts).push_back(playPort);
				}
				else {
					cout << (*device).name << ">>>Get playPort Failed" << endl;
					return false;
				};
				// 打开播放通道
				cout << playPort << endl;
				PLAY_OpenStream(playPort, 0, 0, 1920 * 1080);
				PLAY_SetDecCallBackEx(playPort, DecCBFun, this); 
				//PLAY_SetDecodeThreadNum(playPort,100);
				BOOL bPlayRet = PLAY_Play(playPort, NULL);
			}
			// 窗口句柄传空值，网络库只回调原始数据
			CLIENT_SetRealDataCallBackEx2(lRealPlay, RealDataCallBackEx,(long long) playPorts, REALDATA_FLAG_RAW_DATA);// 用户参数仅传递playPort
			(*device).lRealPlay = lRealPlay;
			
		}
		else {
			printf("CLIENT_RealPlay: failed! Error code: %x.\n", CLIENT_GetLastError());
			return false;
		}
		
		
	}
	
}
void DHConnection::connect(char* &ip, int port, char* &adminName, char* &password,int playPort, void* pUserData,int nChannelID)
{
	
	long long lLogin;
	LLONG lRealPlay;
	NET_DEVICEINFO_Ex deviceInfo = { 0 };
	std::cout << " Login Device ..." << std::endl;
	int error = 0;
	lLogin = CLIENT_LoginEx2(ip, port, adminName, password, EM_LOGIN_SPEC_CAP_TCP, NULL, &deviceInfo, &error);//登陆设备，用户需另选设备。
	
	if (lLogin != 0)
	{
		printf("Login Success ,Start Real Play\n");
		//打开播放通道
		PLAY_OpenStream(playPort, 0, 0, 1920 * 1080);
		PLAY_SetDecCallBackEx(playPort, DecCBFun, pUserData);
		//PLAY_SetDecodeThreadNum(playPort,100);
		BOOL bPlayRet = PLAY_Play(playPort, NULL);
		lRealPlay = CLIENT_RealPlayEx(lLogin, nChannelID, 0);
		if (lRealPlay != 0)
		{
			//窗口句柄传空值，网络库只回调原始数据
			CLIENT_SetRealDataCallBackEx2(lRealPlay, RealDataCallBackEx, playPort, REALDATA_FLAG_RAW_DATA);//用户参数仅传递playPort
			
		}
		else {
			printf("CLIENT_RealPlay: failed! Error code: %x.\n", CLIENT_GetLastError());
		}
	}
	else
	{
		std::string errorStr;
		ConvertLoginError2String(error, errorStr);
		std::cout<<" Login Fail, error reason:" + errorStr<<std::endl;
	}
	Sleep(100000);
	//关闭播放通道，释放资源
	PLAY_Stop(playPort);
	PLAY_CloseStream(playPort);
	//释放网络库
	CLIENT_StopRealPlay(lRealPlay);
	CLIENT_Logout(lLogin);
	CLIENT_Cleanup();
	return;
}
void DHConnection::closePlay() {
	//关闭播放通道，释放资源
	for (auto playport : this->playPort2callBackFun) {
		
		PLAY_Stop(playport.first);
		cout << "Close Play Port :" << playport.first << endl;
		PLAY_CloseStream(playport.first);
		cout << "Close Play Stream :" << playport.first << endl;
	}


	std::vector<Device>::iterator device = this->devices.begin();
	for (; device != this->devices.end(); ++device) {
		//释放网络库
		CLIENT_StopRealPlay((*device).lRealPlay);
		cout << "Close Real Data Play :" << (*device).name << endl;
		CLIENT_Logout((*device).lLogin);
		cout << "Logout Device :" << (*device).name << endl;
	}
	CLIENT_Cleanup();
	cout << "closePlay success!!" << endl;
}
void DHConnection::connects() {
	// 注：如果参数为const vector<int> 需要用const_iterator
	//std::vector<connectConfig>::iterator it = configs.begin();
	// vector<int>::const_iterator iter=v.begin();
	/*for (; it != configs.end(); ++it)
	{
		char*& ip = (*it).ip;
		int port = (*it).port;
		char*& adminName = (*it).adminName;
		char*& password = (*it).password;
		int playPort = (*it).playPort;
		void* callBackFun = (*it).callBackFun;
		int nChannelID = (*it).nChannelID;
		//std::thread th(connect, ip, port, adminName, password, playPort, callBackFun, nChannelID);
		//th.join();
	}*/
}

