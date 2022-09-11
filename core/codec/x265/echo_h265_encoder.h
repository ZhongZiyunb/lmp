/*
 * @Descripttion: 
 * @version: 
 * @Author: zzy
 * @Date: 2022-09-10 16:23:58
 * @LastEditors: 
 * @LastEditTime: 2022-09-10 17:08:56
 */
/*-
* Copyright (c) 2017-2018 Razor, Inc.
*	All rights reserved.
*
* See the file LICENSE for redistribution information.
*/

#ifndef __echo_h265_encoder_h_
#define __echo_h265_encoder_h_

#include <stdint.h>
#include "codec_common.h"

class H265Encoder : public VideoEncoder
{
public:
	H265Encoder();
	virtual ~H265Encoder();

	bool encode(uint8_t *in, int in_size, enum AVPixelFormat pix_fmt, uint8_t *out, int *out_size, int *frame_type, bool request_keyframe = false);

	int get_bitrate() const;

	int get_codec_width() const;
	int get_codec_height() const;

protected:
	void config_param();
	void reconfig_encoder(uint32_t bitrate);
	bool open_encoder();
	void close_encoder();

private:
	uint8_t*		buff_;
	size_t			buff_size_;

	SwsContext*		sws_context_;

	//X264����
	x265_picture*	pic_out_;
	x265_picture*	en_picture_;
	x265_encoder*	en_h_;
	x265_param*	    en_param_;
};

#endif

