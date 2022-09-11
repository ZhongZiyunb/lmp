/*
 * @Descripttion:
 * @version:
 * @Author: zzy
 * @Date: 2022-09-09 09:33:40
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-09-10 18:22:11
 */

#include "./EchoManager.hpp"
#include "./sim_external.h"
#include "./LMPLogger.hpp"
#include <chrono>


enum ECHO_STATE
{
    idle,
    inited,
    connecting,
    connected,
    disconnect
};

void EchoManager::init(uint16_t port, uint32_t conf_min_bitrate, uint32_t conf_start_bitrate, uint32_t conf_max_bitrate)
{

    // @TODO: 打开日志文件
    if(open_log("./echo.log") < 0) {
        logger->log_print(LOG_LEVEL::WARN,"%s","open log echo.log failed!");
        return;
    }
    logger->log_print(LOG_LEVEL::INFO,"%s","start init\n");
    sim_init(port, this, cb_log_write, notify_callback, notify_change_bitrate, notify_state);
    sim_set_bitrates(conf_min_bitrate, conf_start_bitrate, conf_max_bitrate);

    state_ = ECHO_STATE::inited;
}

// 关闭
void EchoManager::finish() {
    logger->log_print(LOG_LEVEL::INFO,"%s","echo finish\n");
	
    if (m_connected){
		if (m_reading){
            _read_thread.stop();
			if (m_viReader != NULL){
				m_viReader->close();
				delete m_viReader;
				m_viReader = NULL;
			}
			m_reading = false;
		}

		disconnect();

		m_connected = false;
	}
    
}


// 网络连接
int EchoManager::connect(int transport_type, int padding, int fec, uint32_t user_id, const char* receiver_ip, uint16_t receiver_port) {
    
    if (state_ != ECHO_STATE::inited)
		return -1;

    logger->log_print(LOG_LEVEL::INFO,"%s","start connecting\n");
	if (sim_connect(user_id, receiver_ip, receiver_port, transport_type, padding, fec) != 0){
		printf("sim connect failed!\n");
		return -2;
	}

	state_ = ECHO_STATE::connecting;

	return 0;
}
// 断开连接
void EchoManager::disconnect() {

    logger->log_print(LOG_LEVEL::INFO,"%s\n","echo disconnect\n");

    if (state_ == ECHO_STATE::connecting || state_ == ECHO_STATE::connected){
		sim_disconnect();

		int i = 0;
		state_ = ECHO_STATE::disconnect;
		while (state_ == ECHO_STATE::disconnect && i++ < 20)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
			// ::Sleep(100);
	}
    m_connected = false;
	state_ = ECHO_STATE::inited;
}

void EchoManager::on_notify(int type, uint32_t val)
{

    // @todo 判断条件
    logger->log_print(LOG_LEVEL::INFO,"%s\n","on notify");
    switch (type)
    {
    case sim_connect_notify:
        if (val == 0)
        {   
            // ::PostMessage(hwnd_, WM_CONNECT_SUCC, NULL, NULL);
            if (state_ == ECHO_STATE::connecting)
                state_ = ECHO_STATE::connected;
            
            OnConnectSucc(0,0);
        }
        else
        {
            // ::PostMessage(hwnd_, WM_CONNECT_FAILED, (WPARAM)val, NULL);
            state_ = ECHO_STATE::inited;

            OnConnectFailed(val,0);

        }

        break;

    case sim_network_timout:
        if (state_ > ECHO_STATE::inited)
        {
            // ::PostMessage(hwnd_, WM_TIMEOUT, (WPARAM)val, NULL);
            state_ = ECHO_STATE::inited;
            OnTimeout(val,0);
        }
        break;

    case sim_disconnect_notify:
        if (state_ == ECHO_STATE::disconnect)
        {
            // ::PostMessage(hwnd_, WM_DISCONNECTED, (WPARAM)val, NULL);
            state_ = ECHO_STATE::inited;
            OnDisconnected(val,0);
        }
        break;

    case sim_start_play_notify:
        // ::PostMessage(hwnd_, WM_START_PLAY, (WPARAM)val, NULL);
        OnStartPlay(val,0);
        break;

    case sim_stop_play_notify:
        // ::PostMessage(hwnd_, WM_STOP_PLAY, (WPARAM)val, NULL);
        OnStopPlay(val,0);
        break;

    case net_interrupt_notify:
        if (state_ == ECHO_STATE::connected)
            OnNetInterrupt(val,0);
            // ::PostMessage(hwnd_, WM_NET_INTERRUPT, (WPARAM)val, NULL);
        break;

    case net_recover_notify:
        if (state_ == ECHO_STATE::connected)
            OnNetRecover(val,0);
            // ::PostMessage(hwnd_, WM_NET_RECOVER, (WPARAM)val, NULL);
        break;

    case sim_fir_notify:
        if (state_ == ECHO_STATE::connected)
            OnFirNotify(val,0);
            // ::PostMessage(hwnd_, WM_FIR_NOTIFY, NULL, NULL);
        break;
    }
}

void EchoManager::on_change_bitrate(uint32_t bitrate_kbps, int lost)
{
    OnChangeBitrate(bitrate_kbps,lost);
}

void EchoManager::on_state(const char *info)
{
    // @TODO
    logger->log_print(LOG_LEVEL::INFO,"%s %s\n","[on state]:",info);

}

// 给 sim_transport 模块的回调函数
static void notify_callback(void *event, int type, uint32_t val)
{
    EchoManager *echo = (EchoManager *)event;
    if (echo != NULL)
        echo->on_notify(type, val);
}

static void notify_change_bitrate(void *event, uint32_t bitrate_kbps, int lost)
{
    EchoManager *echo = (EchoManager *)event;
    if (echo != NULL)
        echo->on_change_bitrate(bitrate_kbps, lost);
}

static void notify_state(void *event, const char *info)
{
    EchoManager *echo = (EchoManager *)event;
    if (echo != NULL)
        echo->on_state(info);
}


//====================================================//
//            回调函数 控制接收器和发送器               //
//====================================================//
void EchoManager::OnConnectSucc(int wparam, int lparam) {
    logger->log_print(LOG_LEVEL::INFO,"%s\n","OnConnectSucc");

    // 连接成功后开启传输线程
    // @todo 区分 MP4 和 YUV
    
		m_viReader = new CFVideoReader("/mnt/d/Workspace/zzy_stock/data.mp4");
		m_viReader->init();
        
		_read_thread.set_video_reader(m_viReader);
        _read_thread.start();
		m_reading = true;
        m_connected = true;
	

}
void EchoManager::OnConnectFailed(int wparam, int lparam) {
    logger->log_print(LOG_LEVEL::WARN,"%s\n","OnConnectFailed");

}
void EchoManager::OnTimeout(int wparam, int lparam) {
    logger->log_print(LOG_LEVEL::INFO,"%s\n","OnTimeout");
    // stop

}
void EchoManager::OnDisconnected(int wparam, int lparam) {
    logger->log_print(LOG_LEVEL::INFO,"%s\n","OnDisconnected");
    // stop

}

void EchoManager::OnStartPlay(int wparam, int lparam) {
    logger->log_print(LOG_LEVEL::INFO,"%s\n","OnStartPlay");

	char data[128];
	uint32_t uid = (uint32_t)wparam;
	sprintf(data, "start play, player = %u\r\n", uid);

    logger->log_print(LOG_LEVEL::INFO,"%s %s","[OnStartPlay]:",data);

	
	m_viPlayer = new CFVideoPlayer();
	m_viPlayer->open();

    _play_thread.set_video_player(m_viPlayer);
    _play_thread.start();
	m_playing = true;

	return ;
}
void EchoManager::OnStopPlay(int wparam, int lparam) {
    logger->log_print(LOG_LEVEL::INFO,"%s\n","OnStopPlay");

    // 停止播放

	char data[128];
	uint32_t uid = (uint32_t)wparam;
	sprintf(data, "stop play, player = %u\r\n", uid);

    logger->log_print(LOG_LEVEL::INFO,"%s %s","[OnStopPlay]:",data);

	if (m_playing){
		_play_thread.stop();
		if (m_viPlayer != NULL){
			m_viPlayer->close();
			delete m_viPlayer;
			m_viPlayer = NULL;
		}
		m_playing = false;
	}

	return ;
}

void EchoManager::OnChangeBitrate(int wparam, int lparam) {
    logger->log_print(LOG_LEVEL::INFO,"%s\n","OnChangeBitrate");

    // 变换码率

	if (m_viReader != NULL){
		uint32_t bitrate = (uint32_t)wparam;
		int lost = (int)lparam;
		bitrate = SU_MAX(bitrate, MIN_VIDEO_BITARE / 1000);
        // @todo 进行码率切换
		// m_viReader->on_change_bitrate(bitrate, lost);
	}
	return ;

}
void EchoManager::OnNetInterrupt(int wparam, int lparam) {
    logger->log_print(LOG_LEVEL::INFO,"%s\n","OnNetInterrupt");
    // 和分辨率转换有关
}
void EchoManager::OnNetRecover(int wparam, int lparam) {
    logger->log_print(LOG_LEVEL::INFO,"%s\n","OnNetRecover");
    // 和分辨率转换有关
}

void EchoManager::OnFirNotify(int wparam, int lparam) {
    logger->log_print(LOG_LEVEL::INFO,"%s\n","OnFirNotify");
    // 和关键帧有关
}

void EchoManager::OnStateInfo(int wparam, int lparam) {
    logger->log_print(LOG_LEVEL::INFO,"%s\n","OnStateInfo");

}