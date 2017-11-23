/* Copyright (c) 2017, Johnyao 
 * All rights reserved.
 *
 * unlock_queue_impl.h
 *
 *  Created on: 2016年5月3日
 *      Author: YAOZHONGCUN
 */

#ifndef YARF_CHANNEL_UNLOCK_QUEUE_IMPL_H_
#define YARF_CHANNEL_UNLOCK_QUEUE_IMPL_H_

#include "yarf/err.h"
#include "yarf/channel/unlock_queue.h"
#include "channel_pack.h"
#include <cstring>

namespace yarf {
namespace channel {

static const uint32_t kUnlockQueueReserve = 12;
static const uint32_t kUnlockQueueMaxCtrlMsg = 512;

// 
// 无锁队列的头部
// 必须初始化好后，才可以交给Queue
// 
class UnlockQueueHead
{
 public:
  uint32_t capacity; // 仅buf的大小
  volatile uint32_t head; // 消息的头指针
  volatile uint32_t tail; // 消息的尾指针
  uint32_t seq;  // 发送消息的序列号，发送方修改和访问，
  uint32_t reserve[kUnlockQueueReserve];  // 预留3个字段使用
  char ctrl_msg[kUnlockQueueMaxCtrlMsg]; // 
  uint32_t ctrl_msg_len; // 忘记设计初衷是做什么的....
 public:
  bool IsValid(uint32_t capacity) {
    return true;
  }
};


// 
// CRecvUnlockQueue 单通的接受队列
//
class CRecvUnlockQueue : public IRecvUnlockQueue
{
public:
	CRecvUnlockQueue()
	: header_(NULL), buff_(NULL)
	{

	}
public:
	int Init(char* mem, uint32_t capacity);

	int Recv(char* data, uint32_t& len);
	int Peek(const char*& data, uint32_t& len);
	int Drop();
private:
  int GetNextPack(uint32_t& cur_head,
    uint32_t &align_pack_size,
    ChannelPackHead* &pack_head);
private:
	UnlockQueueHead* header_;
	char* buff_;
};

// 
// CSendUnlockQueue 单通的发送队列
//
class CSendUnlockQueue : public ISendUnlockQueue
{
public:
	CSendUnlockQueue()
		: header_(NULL), buff_(NULL)
	{

	}
public:
	int Init(char* mem, uint32_t capacity);

	int Send(const char* data, uint32_t len);

	int SetCtrlMsg(const void* data, uint32_t len)
	{
		if ( len > kUnlockQueueMaxCtrlMsg )
		{
			return err::kSpaceLimit;
		}
		memcpy(header_->ctrl_msg, data, len);
		header_->ctrl_msg_len = len;
		return kOk;
	}

	int GetCtrlMsg(void* data, uint32_t& len)
	{
		len = 0;
		if ( len < header_->ctrl_msg_len )
		{
			return err::kSpaceLimit;
		}
		if ( header_->ctrl_msg_len > kUnlockQueueMaxCtrlMsg )
		{
			return err::kUnexpect;
		}

		len = header_->ctrl_msg_len;
		memcpy(data, header_->ctrl_msg, len);
		return kOk;
	}

private:
	UnlockQueueHead* header_;
	char* buff_;
};
// 
// CUnlockQueue 1对1通信的无锁队列 
// 使用CRecvUnlockQueue,CSendUnlockQueue两个队列配合完成
// 
class CUnlockQueue {
 public:
  int Init(char* recv_mem, uint32_t recv_capacity,
    char* send_mem, uint32_t send_capacity,
    uint32_t peer_id);

  ProcId peer_id() {
    return peer_proc_id_;
  }

  int Recv(char* data, uint32_t& len) {
    return recver_.Recv(data, len);
  }

  int Peek(const char*& data, uint32_t& len) {
    return recver_.Peek(data, len);
  }

  int Drop() {
    return recver_.Drop();
  }

  int Send(const char* data, uint32_t len) {
    return sender_.Send(data, len);
  }

  int SetCtrlMsg(const void* data, uint32_t len) {
    return sender_.SetCtrlMsg(data, len);
  }

  int GetCtrlMsg(void* data, uint32_t& len) {
    return sender_.GetCtrlMsg(data, len);
  }

 private:
  CRecvUnlockQueue recver_;
  CSendUnlockQueue sender_;
  uint32_t peer_proc_id_;
};


}  // namespace channel
}  // namespace yarf

#endif  // YARF_CHANNEL_UNLOCK_QUEUE_IMPL_H_
