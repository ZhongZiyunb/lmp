/*
 * @Descripttion: 
 * @version: 
 * @Author: zzy
 * @Date: 2022-09-10 13:13:35
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-09-10 18:36:38
 */

#ifndef VIDEO_READ_THREAD
#define VIDEO_READ_THREAD

#include "BaseThread.hpp"
#include "VideoReader.hpp"
#include "LMPLogger.hpp"
class VideoReadThread : public BaseThread
{
public:
	VideoReadThread();
	~VideoReadThread();

	void set_video_reader(CFVideoReader* rec);

private:
	void run();

	CFVideoReader * _reader;
};

#endif