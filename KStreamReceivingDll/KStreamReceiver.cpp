#include <iostream>
#include "KStreamReceiver.h"

KStreamReceiver::KStreamReceiver()
	: is_receiving(false), last_error(KStreamReceiverError::NO_STREAM_RECEIVER_ERROR),
	receiver(NULL), ffmpeg(), receiveEvent(NULL)
{}

KStreamReceiver::~KStreamReceiver()
{}

void KStreamReceiver::SetFFMPEG(std::string ip, int port)
{
	if (ffmpeg.IsInitialized())
		ffmpeg.Deinitialize();
	ffmpeg.Initialize(ip, port);
}

bool KStreamReceiver::StartReceive()
{
	EndReceive();

	mtx_lock.lock();
	this->is_receiving = true;
	mtx_lock.unlock();

	this->receiver = new std::thread(&KStreamReceiver::ReceiveStream, this);
	if (!this->receiver)
	{
		this->last_error = KStreamReceiverError::THREAD_NOT_CREATED;
		return false;
	}

	return true;
}

void KStreamReceiver::EndReceive()
{
	if (this->receiver)
	{
		mtx_lock.lock();
		this->is_receiving = false;
		mtx_lock.unlock();

		// wait until finish
		this->receiver->join();

		delete this->receiver;
	}
	
	this->receiver = NULL;
}

int KStreamReceiver::GetLastError()
{
	if (KStreamReceiverError::FFMPEG_ERROR)
		return this->ffmpeg.GetLastError();
	else
		return this->last_error;
}

void KStreamReceiver::SetReceiveEvent(void(*receiveEvent)(__in cv::Mat& cv_img))
{
	this->receiveEvent = receiveEvent;
}

void KStreamReceiver::ReceiveStream()
{
	cv::Mat cv_img;
	cv::Mat frame_pool[STREAM_FPS];
	int frame_pool_index = 0;

	while (true)
	{
		mtx_lock.lock();
		bool thread_end = this->is_receiving;
		mtx_lock.unlock();

		// user finish
		if (!thread_end)
		{
			break;
		}

		// read frame
		if (!this->ffmpeg.ReceiveImage(cv_img))
		{
			this->last_error = KStreamReceiverError::FFMPEG_ERROR;
			continue;
		}

		// time stamp
		std::time_t rawtime;
		struct std::tm* timeinfo;
		char timebuf[80];

		SYSTEMTIME time;
		GetLocalTime(&time);
		sprintf(timebuf, "%04d:%02d:%02d-%02d:%02d:%02d:%03d", 
				time.wYear, time.wMonth, time.wDay, 
				time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
		std::string timestr(timebuf);

		//std::time(&rawtime);
		//timeinfo = std::localtime(&rawtime);
		//std::strftime(timebuf, sizeof(timebuf), "%d-%m-%Y %I:%M:%S", timeinfo);
		//std::string timestr(timebuf);
		cv::putText(cv_img, timestr, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar::all(255), 2);

		// event occur
		if (this->receiveEvent)
		{
			frame_pool[frame_pool_index] = cv_img.clone();
			this->receiveEvent(frame_pool[frame_pool_index]);
			frame_pool_index = (frame_pool_index + 1) % STREAM_FPS;
		}
	}
}