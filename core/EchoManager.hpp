/*
 * @Descripttion: 
 * @version: 
 * @Author: zzy
 * @Date: 2022-09-09 09:33:59
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-09-10 15:43:26
 */

#ifndef ECHO_MANAGER_HPP
#define ECHO_MANAGER_HPP

// configer 进行配置
// 
#include "stdint.h"
#include "VideoReader.hpp"
#include "VideoPlayer.hpp"
#include "VideoReadThread.hpp"
#include "VideoPlayThread.hpp"
// 修改自 sim_object.h 中的 Sim_Framework 类
class EchoManager {

public:
    
    // 初始化
    void init(uint16_t port, uint32_t conf_min_bitrate, uint32_t conf_start_bitrate, uint32_t conf_max_bitrate);
	// 关闭
	void finish();

    // 注册回调函数
    void register_cb_func(void * cb_func ,int type);
    
    // 网络连接
    int connect(int transport_type, int padding, int fec, uint32_t user_id, const char* receiver_ip, uint16_t receiver_port);
    void disconnect();

    // 给sim_transport的回调接口
	void on_notify(int type, uint32_t val);
	void on_change_bitrate(uint32_t bitrate_kbps, int lost);
	void on_state(const char* info);

private:
    // 回调函数
    void		OnConnectSucc(int wparam, int lparam);
	void		OnConnectFailed(int wparam, int lparam);
	void		OnTimeout(int wparam, int lparam);
	void		OnDisconnected(int wparam, int lparam);
	
	void		OnStartPlay(int wparam, int lparam);
	void		OnStopPlay(int wparam, int lparam);

	void		OnChangeBitrate(int wparam, int lparam);
	void		OnNetInterrupt(int wparam, int lparam);
	void		OnNetRecover(int wparam, int lparam);

	void		OnFirNotify(int wparam, int lparam);

	void		OnStateInfo(int wparam, int lparam);


	CFVideoReader* m_viReader;
	CFVideoPlayer* m_viPlayer;

	VideoReadThread _read_thread;
	VideoPlayThread _play_thread;


	bool m_connected;
	bool m_playing;
	bool m_reading;

	int	state_; // echo当前的状态


};

static void notify_callback(void *event, int type, uint32_t val);

static void notify_change_bitrate(void *event, uint32_t bitrate_kbps, int lost);

static void notify_state(void *event, const char *info);


#endif