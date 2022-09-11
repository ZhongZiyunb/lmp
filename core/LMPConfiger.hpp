/*
 * @Descripttion: 
 * @version: 
 * @Author: zzy
 * @Date: 2022-09-07 20:46:57
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-09-09 19:01:12
 */



#ifndef LMP_CONFIGER_HPP
#define LMP_CONFIGER_HPP

#include <string>
#include <vector>

class LMPConfiger
{
private:
    /* data */
    int _codec_type; // 编码方式
    std::vector<std::string> _video_list; // 视频文件路径
    
public:

    // 进行文件配置
    bool config(std::string file);
    LMPConfiger(/* args */);
    ~LMPConfiger();
};





#endif