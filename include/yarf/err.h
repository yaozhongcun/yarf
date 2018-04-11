/* Copyright (c) 2017, Johnyao 
 * All rights reserved.
 *
 * err.h
 *
 *  Created on: 2016年5月3日
 *      Author: YAOZHONGCUN
 */

#ifndef YARF_ERR_H_
#define YARF_ERR_H_

#include <stdint.h>  // uint32_t ...
#include <stddef.h>  // NULL

#include <vector>
using std::vector;
#include <string>
using std::string;

namespace yarf {
static const int kOk = 0;

namespace err {
static const int kArg = -1;  // 参数错误
static const int kInternal = -2;  // 内部错误
static const int kUnexpect = -3;  // 理论上看不到错误，返回该错误代替assert
static const int kSpaceLimit = -4;  // 内存不足

static const int kFileIo = -101;  // 内存不足

// 共享内存的错误
static const int kShmGet = -500;
static const int kShmNotExist = -501;
static const int kShmAttach = -502;
static const int kShmDelete = -503;
static const int kShmStat = -504;
static const int kShmDetach = -505;


// -1999 ~ -1000 channel
static const int kChanShmNotExist = -1000;  // ?
static const int kChanHeadInvalid = -1001;  // chan表头数据不合法

static const int kChanFull = -1010;  // channel已满
static const int kChanEmpty = - 1011;  // channel为空
static const int kChanRecvSpaceLimit = -1012;  // 接收缓存大小不足
static const int kChanDstNotExist = -1013;  // channel 目标地址不存在

static const int kChanPackInvalid = -1020;  // channel内包头不合法(致命错误)

// channel配置或者global channel配置，共享内存的大小和约定的不一致
static const int kChanCfgSize = -1030;
static const int kChanCfgInvalid = -1031;  // 配置内容   错误
static const int kChanCfgDuplicateChan = -1032;
// 全局配置中channel配置数量达到kMaxChannelCount
static const int kCfgMaxChan = -1033;

// -2999 ~ -2000 epoll
static const int kEpollCreate = -2000;
static const int kEpollWait = -2001;
static const int kEpollBufFull = -2002;

}  // namespace err
}  // namespace yarf

#endif  // YARF_ERR_H_

