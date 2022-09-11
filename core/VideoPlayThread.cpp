/*
 * @Descripttion: 
 * @version: 
 * @Author: zzy
 * @Date: 2022-09-10 13:15:55
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-09-11 16:31:00
 */
/*-
* Copyright (c) 2017-2018 Razor, Inc.
*	All rights reserved.
*
* See the file LICENSE for redistribution information.
*/

#include "VideoPlayThread.hpp"
#include "sim_external.h"

#define MAX_PIC_SIZE 1024000

FILE * tmp_file  = NULL;
VideoPlayThread::VideoPlayThread()
{
	tmp_file = fopen("/mnt/d/test.h264","wb");

}

VideoPlayThread::~VideoPlayThread()
{
	if(tmp_file != NULL) {
		fclose(tmp_file);
	} 
}

void VideoPlayThread::set_video_player(CFVideoPlayer* player)
{
	_player = player;
}

void VideoPlayThread::run()
{
	uint8_t *data;
	uint8_t payload_type;

	data = (uint8_t*)malloc(MAX_PIC_SIZE * sizeof(uint8_t));

	while (m_run_flag){
		size_t rc = MAX_PIC_SIZE;
		// logger->log_print(LOG_LEVEL::DEBUG,"%s\n","in play thread");
		if (sim_recv_video(data, &rc, &payload_type) == 0 && _player != NULL){
			logger->log_print(LOG_LEVEL::DEBUG,"%s\n","in write");
			_player->write(data, rc, payload_type);
			
			// 检验收到的264对不对 
			if (tmp_file!= NULL) {
				fwrite(data,1,rc,tmp_file);
			}
		}
		else
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	logger->log_print(LOG_LEVEL::DEBUG,"%s\n","video player thread exit");

	free(data);
	m_run_flag = true;
}


