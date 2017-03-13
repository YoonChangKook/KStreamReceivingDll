#include "MyFFMPEGReceiver.h"

MyFFMPEGReceiver::MyFFMPEGReceiver()
	: is_initialized(false), ip("127.0.0.1"), port(8554),
	img_convert_ctx(NULL), context(NULL), ccontext(NULL),
	src_buf(NULL), src_picture(NULL), dst_buf(NULL), dst_picture(NULL)
{}

MyFFMPEGReceiver::~MyFFMPEGReceiver()
{
	if (this->is_initialized)
		Deinitialize();
}

bool MyFFMPEGReceiver::Initialize(std::string ip, int port)
{
	int ret;

	context = avformat_alloc_context();
	ccontext = avcodec_alloc_context3(NULL);
	av_init_packet(&packet);

	/* Initialize libavcodec, and register all codecs and formats. */
	av_register_all();
	avformat_network_init();

	std::string tempUrl("");
	tempUrl.append("rtp://");
	tempUrl.append(ip + ":");
	tempUrl.append(std::to_string(port));
	tempUrl.append("/kstream");

	/* allocate the media context */
	if (avformat_open_input(&context, tempUrl.c_str(), NULL, NULL) != 0){
		this->last_error = MyFFMPEGReceiverError::CANT_ALLOC_FORMAT_CONTEXT;
		return false;
	}

	if (avformat_find_stream_info(context, NULL) < 0){
		this->last_error = MyFFMPEGReceiverError::CANT_FIND_STREAM_INFO;
		return false;
	}

	//search video stream
	for (int i = 0; i<context->nb_streams; i++){
		if (context->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
			video_stream_index = i;
	}

	// set codec
	AVCodec *codec = NULL;
	codec = avcodec_find_decoder(context->streams[video_stream_index]->codec->codec_id);
	if (!codec){
		return false;
	}
	avcodec_get_context_defaults3(ccontext, codec);
	avcodec_copy_context(ccontext, context->streams[video_stream_index]->codec);

	if (avcodec_open2(ccontext, codec, NULL) < 0){
		this->last_error = MyFFMPEGReceiverError::CANT_OPEN_CODEC;
		return false;
	}

	img_convert_ctx = sws_getContext(ccontext->width, ccontext->height, ccontext->pix_fmt,
		ccontext->width, ccontext->height, AV_PIX_FMT_BGR24,
		SWS_BICUBIC, NULL, NULL, NULL);

	int size = avpicture_get_size(ccontext->pix_fmt, ccontext->width, ccontext->height);
	src_buf = (uint8_t*)(av_malloc(size));
	src_picture = av_frame_alloc();
	int size2 = avpicture_get_size(AV_PIX_FMT_BGR24, ccontext->width, ccontext->height);
	dst_buf = (uint8_t*)(av_malloc(size2));
	dst_picture = av_frame_alloc();
	avpicture_fill((AVPicture *)src_picture, src_buf, ccontext->pix_fmt, ccontext->width, ccontext->height);
	avpicture_fill((AVPicture *)dst_picture, dst_buf, AV_PIX_FMT_BGR24, ccontext->width, ccontext->height);

	this->is_initialized = true;

	return true;
}

void MyFFMPEGReceiver::Deinitialize()
{
	av_free_packet(&packet);
	if (src_buf != NULL)
	{
		av_free(src_buf);
		src_buf = NULL;
	}
	if (dst_buf != NULL)
	{
		av_free(dst_buf);
		dst_buf = NULL;
	}
	if (dst_picture != NULL)
	{
		av_free(dst_picture);
		dst_picture = NULL;
	}
	if (src_picture != NULL)
	{
		av_free(src_picture);
		src_picture = NULL;
	}
	avformat_free_context(context);
	avcodec_free_context(&ccontext);
	av_read_pause(context);

	this->is_initialized = false;
}

bool MyFFMPEGReceiver::ReceiveImage(cv::Mat& cv_img)
{
	static AVFrame* frame;

	if (av_read_frame(context, &packet) >= 0)
	{
		int check = 0;
		int result = avcodec_decode_video2(ccontext, src_picture, &check, &packet);

		if (!check)
		{
			this->last_error = MyFFMPEGReceiverError::CANT_DECODE_IMAGE;
			return false;
		}

		sws_scale(img_convert_ctx, src_picture->data, src_picture->linesize, 0, ccontext->height, dst_picture->data, dst_picture->linesize);
		cv::Mat mat(ccontext->height, ccontext->width, CV_8UC3, dst_picture->data[0], dst_picture->linesize[0]);
		cv_img = mat.clone();

		return true;
	}
	else
		return false;
}

int MyFFMPEGReceiver::GetLastError()
{
	return this->last_error;
}

bool MyFFMPEGReceiver::IsInitialized()
{
	return this->is_initialized;
}