/*
 * @Descripttion: 
 * @version: 
 * @Author: zzy
 * @Date: 2022-09-10 13:15:37
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-09-10 18:39:40
 */



#include "VideoReadThread.hpp"
#include "sim_external.h"

#define MAX_PIC_SIZE 1024000


VideoReadThread::VideoReadThread() {

}

VideoReadThread::~VideoReadThread() {
    
}

void VideoReadThread::set_video_reader(CFVideoReader* reader)
{
	_reader = reader;
}

// 线程执行函数
void VideoReadThread::run() {

    uint8_t *data;
	uint8_t payload_type;
	int rc = MAX_PIC_SIZE, size;
	int key;

	data = (uint8_t*)malloc(rc * sizeof(uint8_t));
	
	while (m_run_flag){
		if (_reader != NULL){
			// logger->log_print(LOG_LEVEL::DEBUG,"%s \n","read in!");
			size = _reader->read(data, rc, key, payload_type);
			if (size > 0)
				sim_send_video(payload_type, key, data, size);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	logger->log_print(LOG_LEVEL::DEBUG,"%s \n","read finish!");

	free(data);
	m_run_flag = true;
}

