#ifndef _MY_FFMPEG_RECEIVER_H_
#define _MY_FFMPEG_RECEIVER_H_

#ifdef KSTREAMRECEIVINGDLL_EXPORTS
#define MY_FFMPEG_API __declspec(dllexport)
#else
#define MY_FFMPEG_API __declspec(dllimport)
#endif

#include <string>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libswscale/swscale.h>
}

#include <opencv2/core/core.hpp> // Basic OpenCV structures (cv::Mat)
#include <opencv2/imgproc/imgproc.hpp>

// ffmpeg
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")

// opencv
#pragma comment(lib, "opencv_core2413.lib")
#pragma comment(lib, "opencv_core2413d.lib")
#pragma comment(lib, "opencv_imgproc2413.lib")
#pragma comment(lib, "opencv_imgproc2413d.lib")

#pragma warning(disable:4996)

#define STREAM_FPS		30
#define STREAM_PIX_FMT	AV_PIX_FMT_YUV420P

enum MyFFMPEGReceiverError{
	CANT_ALLOC_FORMAT_CONTEXT = 10,
	CANT_FIND_STREAM_INFO = 11,
	CANT_OPEN_CODEC = 12,
	CANT_DECODE_IMAGE = 13,
	NO_FFMPEG_ERROR = 100
};

class MY_FFMPEG_API MyFFMPEGReceiver
{
public:
	MyFFMPEGReceiver();
	~MyFFMPEGReceiver();

private:
	MyFFMPEGReceiverError last_error;
	bool is_initialized;
	// ffmpeg members
	std::string ip;
	int port;
	SwsContext* img_convert_ctx;
	AVFormatContext* context;
	AVCodecContext* ccontext;
	AVPacket packet;
	int video_stream_index;
	AVFrame *src_picture, *dst_picture;
	uint8_t *src_buf, *dst_buf;

public:
	bool Initialize(std::string ip = "127.0.0.1", int port = 8554);
	void Deinitialize();
	bool ReceiveImage(cv::Mat& cv_img);
	int GetLastError();
	bool IsInitialized();
};

#endif