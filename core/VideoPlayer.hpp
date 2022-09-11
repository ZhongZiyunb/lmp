/*
 * @Descripttion: 
 * @version: 
 * @Author: zzy
 * @Date: 2022-09-10 13:14:25
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-09-10 16:26:59
 */

#ifndef VIDEO_PLAYER_HPP
#define VIDEO_PLAYER_HPP

#include "codec_common.h"
#include <string>

class CFVideoPlayer 
{
public:
	CFVideoPlayer();
	virtual ~CFVideoPlayer();

	bool open();
	void close();

	int write(const void* data, uint32_t size, uint8_t payload_type);
	std::string get_resolution();
private:
	bool		open_;


	uint32_t    decode_data_width_;
	uint32_t    decode_data_height_;

	uint8_t*	data_;
	/*���ӽ���������*/
	int			codec_type_;
	VideoDecoder* decoder_;

	std::string	resolution_;
};

#endif