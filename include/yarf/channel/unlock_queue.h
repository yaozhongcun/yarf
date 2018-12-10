/* Copyright (c) 2017, Johnyao 
 * All rights reserved.
 * 
 * unlock_queue.h
 *
 *  Created on: 2016年5月3日
 *      Author: YAOZHONGCUN
 */

#ifndef YARF_CHANNEL_UNLOCK_QUEUE_H_
#define YARF_CHANNEL_UNLOCK_QUEUE_H_

#include <cstring>
#include "yarf/err.h"
// #include "channel_pack.h"
#include <sys/shm.h>

namespace yarf {
namespace channel {

typedef uint64_t ProcId;  // 进程标识

//
// IRecvUnlockQueue 单通的接受队列
//
class IRecvUnlockQueue {
 public:
  virtual ~IRecvUnlockQueue() {}
 public:
  virtual int Recv(char* data, uint32_t& len) = 0;
  virtual int Peek(const char*& data, uint32_t& len) = 0;
  virtual int Drop() = 0;
};

//
// ISendUnlockQueue 单通的发送队列
//
class ISendUnlockQueue {
 public:
  virtual ~ISendUnlockQueue() {}
 public:
  virtual int Send(const char* data, uint32_t len) = 0;
};

//
// 用户自己分配内存,自己管理队列
// 一般来讲, recv_queue, send_queue会交给不同的线程使用
// 要等相关线程退出后, 先销毁队列,再销毁内存
// reset, true 重制现有队列,删除现有通道上的包
//        false 保留现有队列的信息
int CreateUnlockQueue(
  char* mem, uint32_t capacity,
  IRecvUnlockQueue* &recv_queue, ISendUnlockQueue* &send_queue,
  bool reset = true);

// 统一管理内存的接口 暂未实现
class IUnlockQueuePair {
 public:
  virtual ~IUnlockQueuePair() {}

 public:
  virtual IRecvUnlockQueue* GetRecvPeer() = 0;
  virtual ISendUnlockQueue* GetSendPeer() = 0;
};

// 封装下 暂未实现
IUnlockQueuePair* CreateUnlockQueue(uint32_t capacity);

// 封装下 暂未实现
IUnlockQueuePair* CreateUnlockQueueInShm(key_t shm_key, uint32_t capacity);


}  // namespace channel
}  // namespace yarf

#endif  // YARF_CHANNEL_UNLOCK_QUEUE_H_
