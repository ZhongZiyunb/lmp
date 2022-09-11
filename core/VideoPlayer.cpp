


#include "VideoPlayer.hpp"


#include "echo_h264_encoder.h"
#include "echo_h264_decoder.h"

#include "echo_h265_encoder.h"
#include "echo_h265_decoder.h"
#include "./LMPLogger.hpp"
#include <mutex>

std::mutex mtx;



CFVideoPlayer::CFVideoPlayer()
{

	open_ = false;

	//data_ = (uint8_t *)malloc(sizeof(uint8_t) * MAX_PIC_SIZE);
	resolution_ = "";
}

CFVideoPlayer::~CFVideoPlayer()
{
	this->close();
}

bool CFVideoPlayer::open()
{
	if (open_)
		return true;


	codec_type_ = codec_h264;
	decoder_ = new H264Decoder();
	if (!decoder_->init()){
        // @todo
		// ::ReleaseDC(hwnd_, hdc_);
		// hdc_ = NULL;
	}

	open_ = true;

	return true;
}

void CFVideoPlayer::close()
{
	if (open_){

		open_ = false;

		if (decoder_ != NULL){
			decoder_->destroy();
			delete decoder_;
			decoder_ = NULL;
		}
	}
}

#define SU_MAX(a, b) ((a) > (b) ? (a) : (b))

int CFVideoPlayer::write(const void* data, uint32_t size, uint8_t payload_type)
{
	int width, height;

	int32_t pic_type;

	if (size > 0){

		if (payload_type != codec_type_){
			decoder_->destroy();
			delete decoder_;

			codec_type_ = payload_type;
			if (payload_type == codec_h264){
				decoder_ = new H264Decoder();
			}
			else{
				decoder_ = new H265Decoder();
			}

			decoder_->init();
		}
		logger->log_print(LOG_LEVEL::DEBUG,"%s \n","in decode");
		/*�Ƚ��룬����ʾ*/
		if(decoder_ != NULL && !decoder_->decode((uint8_t *)data, size, &data_, width, height, pic_type)){
			logger->log_print(LOG_LEVEL::DEBUG,"%s \n","in decode ");
			return 0;
		}
		logger->log_print(LOG_LEVEL::DEBUG,"%s \n","out decode ");

		if (decode_data_width_ != width || decode_data_height_ != height){
			// When first time write or the width or height changed
			decode_data_width_ = width;
			decode_data_height_ = height;

			char tmp[64] = { 0 };
			sprintf(tmp, "%dx%d", decode_data_width_, decode_data_height_);
			logger->log_print(LOG_LEVEL::DEBUG,"resolution: %s \n",tmp);
			resolution_ = tmp;
		}

	}

	return 0;
}

std::string	CFVideoPlayer::get_resolution()
{
	std::unique_lock<std::mutex> lock(mtx);
	return resolution_;
}
