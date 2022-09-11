




#ifndef __video_reader_h__
#define __video_reader_h__

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <queue>
#include <chrono>
#include <mutex>
#include "./BaseThread.hpp"
#include "../LMPLogger.hpp"
extern "C" {
	#include "libavformat/avformat.h"
}

struct BaseFrame {

	uint8_t * data;
	int size;
	int key_frame;
	uint8_t payload_type;
	BaseFrame():
		data(NULL),
		size(0)
	{};

	~BaseFrame() {
		if (data != NULL) {
			// free(data);
		}
	}

	void dump_in_frame(uint8_t * d,int sz,int key,uint8_t codec_type) {
		if (data == NULL) {
			data = (uint8_t *)malloc(sz);
		}
		memcpy(data, d, sz);
		size = sz;
		key_frame = key;
		payload_type = codec_type;
	}

};

using namespace std;

// 对视频解复用 提取流信息
class CFVideoDemuxer: public BaseThread
{
public:
	CFVideoDemuxer(){};
	CFVideoDemuxer(char* in) :
		src(in),
		video_fr(30)
	{};

	~CFVideoDemuxer(){};

	// 外部访问frame_cache
	void get_front_cache(BaseFrame &bf_) {
		
		if (!frame_cache.empty()) {
			bf_ = frame_cache.front();
			frame_cache.pop();
		}
		return;
	};

	bool is_cache_empty() {
		return frame_cache.empty();
	}

	int get_frame_rate() {
		return video_fr;
	}

	void mp4_to_h264();

private:

	// MP4 -> H264
	int h264_mp4toannexb(AVFormatContext *fmt_ctx, AVPacket *in, uint8_t* out_data, int& out_data_size, int& key_frame);
	int h264_extradata_to_annexb(const uint8_t *codec_extradata, const int codec_extradata_size, AVPacket *out_extradata, int padding);
	int alloc_and_copy(AVPacket *out, const uint8_t *sps_pps, uint32_t sps_pps_size, const uint8_t *in, uint32_t in_size);

	// 线程启动函数
	void run();

	char * src = NULL;
	queue<BaseFrame> frame_cache;
	int video_fr;



};

class CFVideoReader
{
public:

	CFVideoReader(char *in);

	~CFVideoReader(){};

	// 获取帧率
	int get_frame_interval() {
		return (1000.f/ demuxer.get_frame_rate());
	};
	// 开启demuxer线程
	void init();
	// 从队列中获取
	int	read(void* data, uint32_t size, int& key_frame, uint8_t& payload_type);
	void close();
private:

	// 帧队列
	CFVideoDemuxer		demuxer;
#ifdef WIN_32
	LARGE_INTEGER		prev_timer_;
	LARGE_INTEGER       counter_frequency_;
#endif
    std::chrono::system_clock::time_point            now_time;
    std::chrono::system_clock::time_point            end_time;
	uint64_t			    frame_intval_;
	std::chrono::system_clock::time_point prev ;
};


#endif 