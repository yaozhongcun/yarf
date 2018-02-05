/* Copyright (c) 2017, Johnyao 
 * All rights reserved.
 * 
 * channel_impl.h
 *
 *  Created on: 2016年5月5日
 *      Author: YAOZHONGCUN
 */

#ifndef YARF_CHANNEL_CHANNEL_IMPL_H_
#define YARF_CHANNEL_CHANNEL_IMPL_H_

#include <map>
#include <vector>

#include "yarf/channel/unlock_queue.h"
#include "yarf/channel/channel.h"

namespace yarf {
namespace channel {

typedef std::map<ProcId, CUnlockQueue> UnlockQueueSet;
typedef std::map<ProcId, CUnlockQueue>::iterator UnlockQueueSetIt;

static const uint32_t kMaxQueueRecvPkgCount = 10;

//
// CChannel是一组双通无锁队列的集合
// 一个双通的无锁队列是由两个单通无锁队列组成的,对于使用者来说,他只保存
// 其中一个队列的读端和另一个队列的写端 和他通信的另一个peer则保存相反的
// 一个队列的写端和另一个队列的读端
//
class CChannel : public IChannel, public IChannelInfo {
 public:
  int Init(uint32_t channel_mgr_key, ProcId proc_id, bool enable_log = false);

  int Send(ProcId dst, const char* data, uint32_t len);

  int Recv(ProcId& src, char* data, uint32_t& len);

  int Peek(ProcId& src, const char*& data, uint32_t& len);

  int Drop(ProcId src);

  // 设置控制信息， 只给发送方使用
  int SetCtrlMsg(ProcId dst, const void* data, uint32_t len);
  // 获取控制信息， 只给发送方使用
  int GetCtrlMsg(ProcId dst, void* data, uint32_t& len);

  void GetPeerIds(vector<ProcId>& peer_ids);

 private:
  void MoveToNextQueue();

 private:
  UnlockQueueSet unlock_queues_;
  UnlockQueueSetIt cur_recv_queue_;
  uint32_t cur_queue_recv_pkg_count_;

  ProcId proc_id_;
};

}  // namespace channel
}  // namespace yarf

#endif  // YARF_CHANNEL_CHANNEL_IMPL_H_
