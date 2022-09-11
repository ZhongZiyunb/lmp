/*
 * @Descripttion: 
 * @version: 
 * @Author: zzy
 * @Date: 2022-09-10 13:21:02
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-09-10 14:09:47
 */
/*-
* Copyright (c) 2017-2018 Razor, Inc.
*	All rights reserved.
*
* See the file LICENSE for redistribution information.
*/

#ifndef BASE_THREAD_HPP
#define BASE_THREAD_HPP

#include "pthread.h"
#include <chrono>
#include <thread>

class BaseThread
{
public:
	BaseThread();
	virtual ~BaseThread();

	int start();
	void stop();

protected:	
	static void * thread_work(void * handle);
	virtual void run() = 0;

	bool m_run_flag;

    pthread_t m_thread_handle;
	// HANDLE m_thread_handle;
};

#endif
