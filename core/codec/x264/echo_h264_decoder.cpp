/*
 * @Descripttion: 
 * @version: 
 * @Author: zzy
 * @Date: 2022-09-10 16:23:58
 * @LastEditors: 
 * @LastEditTime: 2022-09-10 18:09:54
 */
/*-
* Copyright (c) 2017-2018 Razor, Inc.
*	All rights reserved.
*
* See the file LICENSE for redistribution information.
*/

#include "echo_h264_decoder.h"

#define BUFF_SIZE (1024 * 1024)
H264Decoder::H264Decoder()
{

}

H264Decoder::~H264Decoder()
{

}

void H264Decoder::set_codec_id()
{
	codec_id_ = AV_CODEC_ID_H264;
}





