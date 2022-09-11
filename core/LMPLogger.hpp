/*
 * @Descripttion: 
 * @version: 
 * @Author: zzy
 * @Date: 2022-09-07 16:13:24
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-09-10 18:25:16
 */

#ifndef LMP_LOGGER_HPP
#define LMP_LOGGER_HPP

// 字体颜色的定义
#define NONE                 "\e[0m"
#define BLACK                "\e[0;30m"
#define L_BLACK              "\e[1;30m"
#define RED                  "\e[0;31m"
#define L_RED                "\e[1;31m"
#define GREEN                "\e[0;32m"
#define L_GREEN              "\e[1;32m"
#define BROWN                "\e[0;33m"
#define YELLOW               "\e[1;33m"
#define BLUE                 "\e[0;34m"
#define L_BLUE               "\e[1;34m"
#define PURPLE               "\e[0;35m"
#define L_PURPLE             "\e[1;35m"
#define CYAN                 "\e[0;36m"
#define L_CYAN               "\e[1;36m"
#define GRAY                 "\e[0;37m"
#define WHITE                "\e[1;37m"
 
#define BOLD                 "\e[1m"
#define UNDERLINE            "\e[4m"
#define BLINK                "\e[5m"
#define REVERSE              "\e[7m"
#define HIDE                 "\e[8m"
#define CLEAR                "\e[2J"
#define CLRLINE              "\r\e[K" //or "\e[1K\r"



#pragma once
#include <sstream>
#include <functional>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <iomanip>

using arg_builder_t = std::function<void(std::ostream &)>;
inline std::string mini_format_impl(const std::string &fmt, const std::vector<arg_builder_t> &arg_builders) {
    std::ostringstream ss;
    size_t start = 0;
    for(size_t i = 0; i < arg_builders.size(); ++i) {
        std::size_t pos = fmt.find("{}", start);
        if (pos == std::string::npos) break;
        ss << fmt.substr(start, pos - start);
        arg_builders[i](ss);
        start = pos + 2 /* length of "{}" */;
    }
    ss << fmt.substr(start);
    return ss.str();
}

template<typename Arg>
inline arg_builder_t mini_format_arg(const Arg &arg) {
    return [&arg](std::ostream &ss) { ss << arg; };
}

template<typename ...Args>
inline std::string mini_format(const std::string &fmt, const Args &...args) {
    return mini_format_impl(fmt, {mini_format_arg(args)...});
}

template<typename ...Args>
inline void mini_print(const std::string &fmt, const Args &...args) {
    printf("%s", mini_format(fmt, args...).c_str());
}

inline void mini_log_info(const std::string &file, int line) {
    auto pos = file.find_last_of("\\/");
    auto now = time(0);
    mini_print("[{}] [thread-{}] [{}:{}] ",
        std::put_time(localtime(&now), "%Y-%m-%d_%H:%M:%S"), 
        std::this_thread::get_id(),
        (pos == std::string::npos ? file : file.substr(pos + 1)), line);
}

#define MINI_LOG(...) do { mini_log_info(__FILE__, __LINE__); mini_print(__VA_ARGS__); } while(0)


enum LOG_TYPE {
    LOG,
    TERMINAL
};

enum LOG_LEVEL {
    INFO,
    DEBUG,
    WARN
};


#include <stdio.h>
#include <string>
#include <iostream>
#include <mutex>
// LMP日志类
class LMPLogger
{
private:

    int _out_put_type;
    FILE* _log_file;
    std::mutex mtx;
    

public:
    LMPLogger():
        _out_put_type(LOG_TYPE::TERMINAL)
    {};
    ~LMPLogger(){};

    void set_output(int type);
    
    int log_print(int level, const char* fmt, ...);
    // 单例模式 C++对局部静态有优化 待验证
    static LMPLogger *instance()  
    {  
        static LMPLogger ins;
        return &ins;  
    }  
};


// 给razor log的回调接口
int open_log(const char* filename);
void close_log();
const char* get_time_str(char *date_str);
static const char* get_file_name(const char* pathname);
int cb_log_write(int level, const char* file, int line, const char* fmt, va_list vl);

extern LMPLogger * logger ;


#endif