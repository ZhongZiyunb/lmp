/*
 * @Descripttion: 
 * @version: 
 * @Author: zzy
 * @Date: 2022-09-10 13:21:13
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-09-11 12:04:43
 */


#include "./BaseThread.hpp"


BaseThread::BaseThread() :  m_run_flag(false)
{
}

BaseThread::~BaseThread()
{
	stop();
}
int BaseThread::start()
{
	// @todo 开启线程
    if (m_run_flag == true) {
        return 0;
    }
	m_run_flag = true;
    
    pthread_t thread;
	if (pthread_create(&thread, NULL, BaseThread::thread_work, this) == 0)
        m_thread_handle = thread;
    
    
	pthread_detach(thread);

	return 0;
}

void BaseThread::stop()
{

    if (m_run_flag == false) {
        return;
    }

	m_run_flag = false;

	int i = 0; 
	while(i < 20 && !m_run_flag)
	{
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
		// Sleep(100);
		i ++;
	}

    // LINUX 没有自动收回线程API 留给操作系统处理    
    pthread_join(m_thread_handle, NULL);
    

}

void* BaseThread::thread_work(void * handle) {

    BaseThread* body = (BaseThread*) handle;

    if (body != NULL) {

        body->run();
        
        return NULL;
    }
    else {
        return NULL;
    }

}

