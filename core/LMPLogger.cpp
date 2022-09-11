/*
 * @Descripttion: 
 * @version: 
 * @Author: zzy
 * @Date: 2022-09-07 16:13:41
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-09-10 18:25:47
 */
#include "./LMPLogger.hpp"
#include "sys/time.h"
#include "stdarg.h"
#include "string.h"
#include <chrono>
#include "time.h"
#define MAXLEN 1024

static FILE * log_file = NULL;
static std::mutex mtx;

LMPLogger * logger = LMPLogger::instance();

void LMPLogger::set_output(int type) {
        
        switch (type)
        {
        case LOG_TYPE::TERMINAL:            
                
                break;
        case LOG_TYPE::LOG:

                break;
        default:

            printf("INVALID INPUT TYPE");
            break;
        }    
}

int LMPLogger::log_print(int level, const char* fmt, ...) {
	
	std::unique_lock<std::mutex> lock(mtx);

 	char str[MAXLEN];
	int i = 0;
	va_list vArgList;
	va_start(vArgList, fmt);
	i = vsnprintf(str, MAXLEN, fmt, vArgList);
	va_end(vArgList);
	// printf("%s",str);

    // 日志分等级输出
    switch (level)
    {
    case LOG_LEVEL::INFO :
        
        printf(GREEN "[INFO] %s" NONE,str);
		
        break;
    case LOG_LEVEL::DEBUG :

        printf(YELLOW "[DEBUG] %s" NONE,str);
        break;
    case LOG_LEVEL::WARN :

        printf(RED "[WARN] %s" NONE,str);
        break;
    default:
        break;
    }
	return 0;
}

int open_log(const char* filename)
{
	if (log_file != NULL || filename == NULL)
		return -1;

	log_file = (FILE *)calloc(1, sizeof(FILE));

	// char* path = get_fullexepath();
	// if (path == NULL)
	// 	return -1;

	// sprintf(log_file->filename, "%s\\%s", path, filename);

	log_file = fopen(filename, "w");
	if (log_file == NULL){
		printf("open %s failed!\r\n", filename);
		return -1;
	}

	return 0;
}

void close_log()
{
	if (log_file != NULL){
		fflush(log_file);
		fclose(log_file);
		log_file = NULL;

		free(log_file);
		log_file = NULL;
	}
}




const char* get_time_str(char *date_str)
{
    struct tm* tm_now;
	struct timeval tv;
	gettimeofday(&tv, NULL);

	time_t now = tv.tv_sec;
	//localtime_r(&tm_now, &now);
	tm_now = localtime(&now);


	sprintf(date_str, "%04d-%02d-%02d %02d:%02d:%02d.%3ld", tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday,
		tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec, tv.tv_usec / 1000);

	return date_str;
}

static const char* get_file_name(const char* pathname)
{
	size_t size;
	if (pathname == NULL)
		return pathname;

	size = strlen(pathname);

	char *pos = (char *)pathname + size;
	while (*pos != '\\' && pos != pathname)
		pos--;

	if (pos == pathname)
		return pathname;
	else
		return pos + 1;
}


#define DATE_STR_SIZE 64
int cb_log_write(int level, const char* file, int line, const char* fmt, va_list vl) {
    
    char date_str[DATE_STR_SIZE];
	if (log_file != NULL){
		// TODO: 加锁
		std::unique_lock<std::mutex> lock(mtx);

		fprintf(log_file, "%s %s:%d ", get_time_str(date_str), get_file_name(file), line);
		vfprintf(log_file, fmt, vl);
		fflush(log_file);

	}

	return 0;
}

/*

// 字符串解析 限定1024个字符
    char str[MAXLEN];
	int i = 0;
	va_list vArgList;
	va_start(vArgList, fmt);
	i = vsnprintf(str, MAXLEN, fmt, vArgList);
	va_end(vArgList);
	printf("%s",str);

    // 日志分等级输出
    switch (level)
    {
    case LOG_LEVEL::INFO :
        
        // printf("[]",,str);
        break;
    case LOG_LEVEL::DEBUG :


        break;
    case LOG_LEVEL::WARN :


        break;
    default:
        break;
    }
    
*/