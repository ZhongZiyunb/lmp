

/*
 * @Descripttion: 
 * @version: 
 * @Author: zzy
 * @Date: 2022-09-10 13:13:35
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-09-11 11:08:42
 */

#ifndef VIDEO_PLAY_THREAD
#define VIDEO_PLAY_THREAD

#include "BaseThread.hpp"
#include "VideoPlayer.hpp"
#include "./LMPLogger.hpp"
class VideoPlayThread : public BaseThread
{
public:
	VideoPlayThread();
	~VideoPlayThread();

	void set_video_player(CFVideoPlayer* player);

private:
	void run();

	CFVideoPlayer * _player;
};

#endif