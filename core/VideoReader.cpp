/*
 * @Descripttion: 
 * @version: 
 * @Author: zzy
 * @Date: 2022-09-10 13:13:52
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-09-11 16:21:16
 */




#include "VideoReader.hpp"
#include <chrono>
#include <iostream>

extern "C" {
	#include "libavformat/avformat.h"
	#include "libavformat/avio.h"

}

#ifndef AV_WB32
#   define AV_WB32(p, val) do {                 \
        uint32_t d = (val);                     \
        ((uint8_t*)(p))[3] = (d);               \
        ((uint8_t*)(p))[2] = (d)>>8;            \
        ((uint8_t*)(p))[1] = (d)>>16;           \
        ((uint8_t*)(p))[0] = (d)>>24;           \
    } while(0)
#endif

#ifndef AV_RB16
#   define AV_RB16(x)                           \
    ((((const uint8_t*)(x))[0] << 8) |          \
      ((const uint8_t*)(x))[1])
#endif


#define MAX_PIC_SIZE 4096000

// ================== reader =======================
/*load ffmpeg lib*/
// #pragma comment(lib, "avcodec.lib")
// #pragma comment(lib, "avutil.lib")
// #pragma comment(lib, "swscale.lib")
// #pragma comment(lib, "avformat.lib")


std::mutex reader_mtx;
std::mutex demuxer_mtx;

FILE* dst = NULL;
CFVideoReader::CFVideoReader(char *in)
{
	
	if (dst == NULL) {
		dst = fopen("/mnt/d/Workspace/zzy_stock/echo.h264", "wb");
	}
	else {
		printf("dst open failed\n");
	}
	CFVideoDemuxer d(in);
	demuxer = d;
};

void CFVideoReader::init() {
	
	// QueryPerformanceFrequency(&counter_frequency_);
	// QueryPerformanceCounter(&prev_timer_);


	prev = std::chrono::high_resolution_clock::now();
	demuxer.start();

}



int	CFVideoReader::read(void* data, uint32_t size, int& key_frame, uint8_t& payload_type) {

	now_time = std::chrono::high_resolution_clock::now();
	uint64_t dur = std::chrono::duration_cast<std::chrono::milliseconds>(now_time-prev).count();
	// 控制发送帧率
	if (get_frame_interval() > dur) {
		// logger->log_print(LOG_LEVEL::INFO,"%s \n", "interval wait");
		return 0;
	}
	prev = now_time;
	logger->log_print(LOG_LEVEL::INFO,"interval:%d \n",dur);
	// 如果解复用器队列不为空才读
	if (!demuxer.is_cache_empty()){
		
		std::unique_lock<std::mutex> lock(reader_mtx);
		BaseFrame bf;
		demuxer.get_front_cache(bf);
	
		memcpy(data, bf.data, bf.size);
		size = bf.size;
		key_frame = bf.key_frame;
		payload_type = bf.payload_type; // H264编码的 codec_type
		free(bf.data);
		return size;
	}
	else {
		return 0;
	}
};


void CFVideoReader::close() {

	demuxer.stop();
	if (!dst) {
		fclose(dst);
	}
}



// ================== demuxer =======================
// 原有的ffmpeg版本没有，对于VS添加动态链接库又还不熟,先把源码拷过来用着
// static AVPacket *av_packet_alloc()
// {
// 	AVPacket *pkt = (AVPacket *)av_mallocz(sizeof(AVPacket));
// 	if (!pkt)
// 		return pkt;

// 	av_init_packet(pkt);

// 	return pkt;
// }


// static void av_packet_free(AVPacket **pkt)
// {
// 	if (!pkt || !*pkt)
// 		return;

// 	av_packet_unref(*pkt);
// 	av_freep(pkt);
// }



int CFVideoDemuxer::alloc_and_copy(AVPacket *out,
	const uint8_t *sps_pps, uint32_t sps_pps_size,
	const uint8_t *in, uint32_t in_size)
{
	uint32_t offset = out->size;
	uint8_t nal_header_size = 4;
	int err;

	err = av_grow_packet(out, sps_pps_size + in_size + nal_header_size);
	if (err < 0)
		return err;

	if (sps_pps)
		memcpy(out->data + offset, sps_pps, sps_pps_size);
	memcpy(out->data + sps_pps_size + nal_header_size + offset, in, in_size);
	if (!offset) {
		AV_WB32(out->data + sps_pps_size, 1);
	}
	else {
		(out->data + offset + sps_pps_size)[0] =
			(out->data + offset + sps_pps_size)[1] = 0;
		(out->data + offset + sps_pps_size)[2] = 1;
	}

	return 0;
}



int CFVideoDemuxer::h264_extradata_to_annexb(const uint8_t *codec_extradata, const int codec_extradata_size, AVPacket *out_extradata, int padding)
{
	uint16_t unit_size = 0;
	uint64_t total_size = 0;
	uint8_t *out = NULL;
	uint8_t unit_nb = 0;
	uint8_t sps_done = 0;
	uint8_t sps_seen = 0;
	uint8_t pps_seen = 0;
	uint8_t sps_offset = 0;
	uint8_t pps_offset = 0;

	/**
	* AVCC
	* bits
	*  8   version ( always 0x01 )
	*  8   avc profile ( sps[0][1] )
	*  8   avc compatibility ( sps[0][2] )
	*  8   avc level ( sps[0][3] )
	*  6   reserved ( all bits on )
	*  2   NALULengthSizeMinusOne    // 这个值是（前缀长度-1），值如果是3，那前缀就是4，因为4-1=3
	*  3   reserved ( all bits on )
	*  5   number of SPS NALUs (usually 1)
	*
	*  repeated once per SPS:
	*  16     SPS size
	*
	*  variable   SPS NALU data
	*  8   number of PPS NALUs (usually 1)
	*  repeated once per PPS
	*  16    PPS size
	*  variable PPS NALU data
	*/

	const uint8_t *extradata = codec_extradata + 4; //extradata存放数据的格式如上，前4个字节没用，所以将其舍弃
	static const uint8_t nalu_header[4] = { 0, 0, 0, 1 }; //每个H264裸数据都是以 0001 4个字节为开头的

	extradata++;//跳过一个字节，这个也没用 

	sps_offset = pps_offset = -1;

	/* retrieve sps and pps unit(s) */
	unit_nb = *extradata++ & 0x1f; /* 取 SPS 个数，理论上可以有多个, 但我没有见到过多 SPS 的情况*/
	if (!unit_nb) {
		goto pps;
	}
	else {
		sps_offset = 0;
		sps_seen = 1;
	}

	while (unit_nb--) {
		int err;

		unit_size = AV_RB16(extradata);
		total_size += unit_size + 4; //加上4字节的h264 header, 即 0001
		if (total_size > INT_MAX - padding) {
			av_log(NULL, AV_LOG_ERROR,
				"Too big extradata size, corrupted stream or invalid MP4/AVCC bitstream\n");
			av_free(out);
			return AVERROR(EINVAL);
		}

		//2:表示上面 unit_size 的所占字结数
		//这句的意思是 extradata 所指的地址，加两个字节，再加 unit 的大小所指向的地址
		//是否超过了能访问的有效地址空间
		if (extradata + 2 + unit_size > codec_extradata + codec_extradata_size) {
			av_log(NULL, AV_LOG_ERROR, "Packet header is not contained in global extradata, "
				"corrupted stream or invalid MP4/AVCC bitstream\n");
			av_free(out);
			return AVERROR(EINVAL);
		}

		//分配存放 SPS 的空间
		if ((err = av_reallocp(&out, total_size + padding)) < 0)
			return err;

		memcpy(out + total_size - unit_size - 4, nalu_header, 4);
		memcpy(out + total_size - unit_size, extradata + 2, unit_size);
		extradata += 2 + unit_size;
	pps:
		//当 SPS 处理完后，开始处理 PPS
		if (!unit_nb && !sps_done++) {
			unit_nb = *extradata++; /* number of pps unit(s) */
			if (unit_nb) {
				pps_offset = total_size;
				pps_seen = 1;
			}
		}
	}

	//余下的空间清0
	if (out){
		memset(out + total_size, 0, padding);
	}

	if (!sps_seen)
		av_log(NULL, AV_LOG_WARNING,
		"Warning: SPS NALU missing or invalid. "
		"The resulting stream may not play.\n");

	if (!pps_seen)
		av_log(NULL, AV_LOG_WARNING,
		"Warning: PPS NALU missing or invalid. "
		"The resulting stream may not play.\n");

	out_extradata->data = out;
	out_extradata->size = total_size;

	return 0;
}


int CFVideoDemuxer::h264_mp4toannexb(AVFormatContext *fmt_ctx, AVPacket *in, uint8_t* out_data, int& out_data_size, int& key_frame) {

	AVPacket *out = NULL;
	AVPacket spspps_pkt;

	int len;
	uint8_t unit_type;
	int32_t nal_size;
	uint32_t cumul_size = 0;
	const uint8_t *buf;
	const uint8_t *buf_end;
	int            buf_size;
	int ret = 0, i;

	out = av_packet_alloc();
	
	
	buf = in->data;
	buf_size = in->size;
	buf_end = in->data + in->size;
	// printf("in pts:%li\n", in->pts);

	key_frame = 0;
	
	do {
		ret = AVERROR(EINVAL);
		//因为每个视频帧的前 4 个字节是视频帧的长度
		//如果buf中的数据都不能满足4字节，所以后面就没有必要再进行处理了
		if (buf + 4 > buf_end)
			goto fail;

		//将前四字节转换成整型,也就是取出视频帧长度
		for (nal_size = 0, i = 0; i<4; i++)
			nal_size = (nal_size << 8) | buf[i];

		buf += 4; //跳过4字节（也就是视频帧长度），从而指向真正的视频帧数据 
		unit_type = *buf & 0x1f; //视频帧的第一个字节里有NAL TYPE

		//如果视频帧长度大于从 AVPacket 中读到的数据大小，说明这个数据包肯定是出错了
		if (nal_size > buf_end - buf || nal_size < 0)
			goto fail;

		/* prepend only to the first type 5 NAL unit of an IDR picture, if no sps/pps are already present */
		if (unit_type == 5) {
			key_frame = 1;
			printf("key frame \n");
			//在每个I帧之前都加 SPS/PPS
			h264_extradata_to_annexb(fmt_ctx->streams[in->stream_index]->codecpar->extradata,
				fmt_ctx->streams[in->stream_index]->codecpar->extradata_size,
				&spspps_pkt,
				AV_INPUT_BUFFER_PADDING_SIZE);

			if ((ret = alloc_and_copy(out,
				spspps_pkt.data, spspps_pkt.size,
				buf, nal_size)) < 0)
				goto fail;
		}
		else {
			
			if ((ret = alloc_and_copy(out, NULL, 0, buf, nal_size)) < 0)
				goto fail;
		}


		//len = fwrite(out->data, 1, out->size, dst_fd);
		//TODO:这边还要进一步处理 保证安全性
		memcpy(out_data + out_data_size, out->data, out->size);
		out_data_size += out->size;
		
	next_nal:
		buf += nal_size;
		cumul_size += nal_size + 4;//s->length_size;
	} while (cumul_size < buf_size);

fail:
	av_packet_free(&out);

	return ret;
}

void CFVideoDemuxer::mp4_to_h264() {

	logger->log_print(LOG_LEVEL::DEBUG,"%s\n","mp4_to_h264");
	if (src == NULL) return;

	AVFormatContext * fmt_ctx = NULL;
	AVPacket pkt;

	uint8_t * out_data = (uint8_t*)malloc(MAX_PIC_SIZE*sizeof(uint8_t));
	int out_data_size = 0;
	int key_frame = 0;
	int ret = 0;
	int video_stream_index = -1;
	char errors[1024];
	
	if ((ret = avformat_open_input(&fmt_ctx, src, NULL, NULL)) < 0){
		av_strerror(ret, errors, 1024);
		av_log(NULL, AV_LOG_DEBUG, "Could not open source file: %s, %d(%s)\n",
			src,
			ret,
			errors);
		return;
	}

	/*dump input information*/
	// av_dump_format(fmt_ctx, 0, src, 0);

	/*initialize packet*/
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;

	/*find best video stream*/
	video_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (video_stream_index < 0){
		av_log(NULL, AV_LOG_DEBUG, "Could not find %s stream in input file %s\n",
			av_get_media_type_string(AVMEDIA_TYPE_VIDEO),
			src);
		return;
	}
	
	video_fr = fmt_ctx->streams[video_stream_index]->avg_frame_rate.num;
	
	printf("video frame rate: %d\n", video_fr);
	/*read frames from media file*/
	int cnt = 0;
	while (av_read_frame(fmt_ctx, &pkt) >= 0){

		if (pkt.stream_index == video_stream_index){
			
			out_data_size = 0;
			h264_mp4toannexb(fmt_ctx, &pkt,out_data,out_data_size,key_frame);
			if (out_data_size > 0) {
				// AutoSpLock auto_lock(lock_); 
				std::unique_lock<std::mutex> lock(reader_mtx);// 保证读写安全
				BaseFrame bf;
				bf.dump_in_frame(out_data,out_data_size,key_frame,35);
				fwrite(out_data,1,out_data_size,dst);
				frame_cache.push(bf);
				cnt ++;
			}
			if (cnt >= 299) break;
		}

		//release pkt->data
		av_packet_unref(&pkt);
	}
	printf("cnt: %i \n", cnt);
	/*close input media file*/
	avformat_close_input(&fmt_ctx);
}


void CFVideoDemuxer::run() {
	
	// av_register_all();
	mp4_to_h264();

}

