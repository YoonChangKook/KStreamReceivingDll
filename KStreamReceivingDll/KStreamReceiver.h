#ifndef _K_STREAM_RECEIVER_H_
#define _K_STREAM_RECEIVER_H_

#ifdef KSTREAMRECEIVINGDLL_EXPORTS
#define K_STREAM_RECEIVING_API __declspec(dllexport)
#else
#define K_STREAM_RECEIVING_API __declspec(dllimport)
#endif

#include <mutex>
#include <thread>
#include <ctime>

#include <opencv2/core/core.hpp> // Basic OpenCV structures (cv::Mat)
#include <opencv2/highgui/highgui.hpp>

#include "MyFFMPEGReceiver.h"

// opencv
#pragma comment(lib, "opencv_core2413.lib")
#pragma comment(lib, "opencv_core2413d.lib")
#pragma comment(lib, "opencv_highgui2413.lib")
#pragma comment(lib, "opencv_highgui2413d.lib")

enum KStreamReceiverError{
	THREAD_NOT_CREATED = 0,
	FFMPEG_ERROR = 9,
	NO_STREAM_RECEIVER_ERROR = 100
};

class K_STREAM_RECEIVING_API KStreamReceiver
{
public:
	KStreamReceiver();
	~KStreamReceiver();

private:
	bool is_receiving;
	enum KStreamReceiverError last_error;
	// receive stream from remote server
	std::mutex mtx_lock;
	std::thread* receiver;
	// ffmpeg members
	MyFFMPEGReceiver ffmpeg;
	// stream receiver
	void ReceiveStream();

	// event occur when receive image successfully
	void(*receiveEvent)(__in cv::Mat& cv_img);

public:
	void SetFFMPEG(std::string ip = "127.0.0.1", int port = 8554);
	bool StartReceive();
	void EndReceive();
	int GetLastError();
	/*
	set event to get image when stream receiver succesfully receive.
	*/
	void SetReceiveEvent(void(*receiveEvent)(__in cv::Mat& cv_img));
};

#endif