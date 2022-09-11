/*
 * @Descripttion: 
 * @version: 
 * @Author: zzy
 * @Date: 2022-09-02 23:02:39
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-09-10 13:11:15
 */


// 为了让echo实验能够在更多机子上运行，lmp去除了windowsAPI部分，
// 使得实验代码面向开发者，将实验配置进行抽象，方便网络时延的测量
// 结合时延测量，对代码结构进行重新设计。

// 配置管理者 LMPConfiger
// 根据文件进行实验配置写入

// 时延管理者 LatencyManager
// 通过一个统一对象管理时延数据

// 日志管理者 LMPLogger

// 网络管理者 NetManager

// 会话管理者 SessionManager

// 修改sim_framework的notify 改为回调函数注册
// 整体架构上
// 读入文件后

#include <stdio.h>
#include <stdarg.h>
#include <iostream>  
#include "./LMPLogger.hpp"
#include "./LMPConfiger.hpp"
#include "./sim_external.h"
#include "./EchoManager.hpp"
#define MAXLEN 1024
using namespace std;

#define MIN_VIDEO_BITARE (32 * 1000)
#define START_VIDEO_BITRATE (800 * 1000)
#define MAX_VIDEO_BITRATE (1000 * 1000)


#include <chrono>
int main()
{

    // 基本参数
    int transport_type = bbr_transport; 
    int padding = 1; 
    int fec = 0;
    uint32_t user_id = 1111; 
    const char* receiver_ip = "127.0.0.1";
    uint16_t receiver_port = 16008;

    // 回环管理者
    EchoManager echo_manager;

    // 回环初始化    
    echo_manager.init(16008, MIN_VIDEO_BITARE, START_VIDEO_BITRATE, MAX_VIDEO_BITRATE);
    
    // 连接对端
    echo_manager.connect(1, padding, fec, user_id, receiver_ip, receiver_port);
    
    
    while (1) {}
    // @todo 写一个循环进行控制
    
    // 开启网络会话主线程
    echo_manager.finish();
	return 0;
}
