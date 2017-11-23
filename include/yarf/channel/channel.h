/* Copyright (c) 2017, Johnyao 
 * All rights reserved.
 *
 * channel.h
 *
 *  Created on: 2016年5月3日
 *      Author: YAOZHONGCUN
 */

#ifndef YARF_CHANNEL_CHANNEL_H_
#define YARF_CHANNEL_CHANNEL_H_

#include <vector>
using std::vector;

#include "yarf/err.h"
#include "yarf/channel/unlock_queue.h"

namespace yarf {
namespace channel {

// channel接口
class IChannel {
  // 工厂方法 创建一个channel实例。
 public:
  static IChannel* Create();

 public:
  virtual ~IChannel() {}

 public:
  virtual int Init(uint32_t channel_mgr_key, ProcId proc_id,
    bool enable_log = false) = 0;

  virtual int Send(ProcId dst, const char* data, uint32_t len) = 0;

  virtual int Recv(ProcId& src, char* data, uint32_t& len) = 0;

  virtual int Peek(ProcId& src, const char*& data, uint32_t& len) = 0;

  virtual int Drop(ProcId src) = 0;
};

class IChannelInfo {
 public:
  virtual ~IChannelInfo() {}

 public:
  virtual void GetPeerIds(vector<ProcId>& peer_ids) = 0;
};

}  // namespace channel

}  // namespace yarf

#endif  // YARF_CHANNEL_CHANNEL_H_
