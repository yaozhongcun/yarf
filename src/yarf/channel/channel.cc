/* Copyright (c) 2017, Johnyao 
 * All rights reserved.
 * 
 * channel.cc
 *
 *  Created on: 2016年5月3日
 *      Author: YAOZHONGCUN
 */

#include <iostream>
using std::cout;
using std::endl;

#include "yarf/channel/channel.h"
#include "yarf/memory/shm.h"

#include "./channel_config.h"
#include "./unlock_queue_impl.h"
#include "./channel_impl.h"

using yarf::memory::CShm;

using yarf::channel::IChannel;
using yarf::channel::IChannelInfo;
using yarf::channel::CChannel;
using yarf::channel::CUnlockQueue;
using yarf::channel::GlobalConfig;
using yarf::channel::ChanConfig;

IChannel* IChannel::Create() {
  CChannel* instance = new CChannel();
  return instance;
}

void CChannel::GetPeerIds(vector<ProcId>& peer_ids) {
  UnlockQueueSetIt it = unlock_queues_.begin();
  for ( ; it != unlock_queues_.end(); ++it) {
    peer_ids.push_back(it->first);
  }
}

int CChannel::Init(uint32_t channel_mgr_key,
  const ProcId proc_id, bool enable_log ) {
  yarf::channel::GlobalConfig* global_channel_config = NULL;
  CShm shm;
  bool exist = true;
  int ret_code = shm.InitByKey(
    channel_mgr_key, sizeof(yarf::channel::GlobalConfig), exist);
  if ( ret_code != 0 ) {
    return ret_code;
  }

  if (shm.size() != sizeof(yarf::channel::GlobalConfig))) {
    return yarf::err::kChanCfgSize;
  }
  global_channel_config =
    reinterpret_cast<yarf::channel::GlobalConfig*>(shm.content());

  if ( !global_channel_config->IsValid() ) {
    return yarf::err::kChanCfgInvalid;
  }

  vector<yarf::channel::ChanConfig> channel_config;
  if ( 0 != ( ret_code =
    global_channel_config->GetChansByProcId(proc_id, channel_config))
    ) {
    return ret_code;
  }

  for (size_t idx = 0; idx < channel_config.size(); ++idx) {
    const yarf::channel::ChanConfig* config = &(channel_config[idx]);
    ProcId peer_id = config->peer_id[1];

    yarf::channel::ChanConfig image_config;  // 镜像配置
    if ( proc_id == config->peer_id[1] ) {
      image_config.recv_shm_id = config->send_shm_id;
      image_config.send_shm_id = config->recv_shm_id;
      image_config.recv_size = config->send_size;
      image_config.send_size = config->recv_size;

      peer_id = config->peer_id[0];
      config = &image_config;
    } else if (proc_id != config->peer_id[0]) {
      return yarf::err::kUnexpect;
    }

    if ( unlock_queues_.find(peer_id) != unlock_queues_.end() ) {
      return yarf::err::kChanCfgDuplicateChan;
    }

    CShm send_shm;
    CShm recv_shm;
    if ( 0 != (ret_code =
      send_shm.InitById(config->send_shm_id, config->send_size))) {
      return ret_code;
    }
    if ( 0 != (ret_code =
      recv_shm.InitById(config->recv_shm_id, config->recv_size))) {
      return ret_code;
    }

    CUnlockQueue queue;
    if ( 0 != (ret_code = queue.Init( reinterpret_cast<char*>(recv_shm.content()), config->recv_size,
                reinterpret_cast<char*>(send_shm.content()), config->send_size, peer_id))
      ) {
      return ret_code;
    }

    unlock_queues_[peer_id] = queue;
  }

  cur_recv_queue_ = unlock_queues_.begin();
  cur_queue_recv_pkg_count_ = 0;
  proc_id_ = proc_id;

  return yarf::kOk;
}


void CChannel::MoveToNextQueue() {
  if ( unlock_queues_.end() ==  cur_recv_queue_ )  {
    //  异常处理
    cur_recv_queue_ = unlock_queues_.begin();
    cur_queue_recv_pkg_count_ = 0;
    // cout << "move to next to begin: " << cur_recv_queue_->second.peer_id() << endl;
    return;
  }

  ++cur_recv_queue_;
  if ( unlock_queues_.end() ==  cur_recv_queue_ ) {
    cur_recv_queue_ = unlock_queues_.begin();
  }
  // cout << "move to next: " << cur_recv_queue_->second.peer_id() << endl;
  cur_queue_recv_pkg_count_ = 0;
}

int CChannel::Send(ProcId dst, const char* data, uint32_t len) {
  UnlockQueueSetIt dst_it = unlock_queues_.find(dst);
  if ( unlock_queues_.end() == dst_it ) {
    return yarf::err::kChanDstNotExist;
  }

  CUnlockQueue& queue = dst_it->second;
  return queue.Send(data, len);
}

int CChannel::SetCtrlMsg(ProcId dst, const void* data, uint32_t len) {
  UnlockQueueSetIt dst_it = unlock_queues_.find(dst);
  if ( unlock_queues_.end() == dst_it ) {
    return yarf::err::kChanDstNotExist;
  }

  CUnlockQueue& queue = dst_it->second;
  return queue.SetCtrlMsg(data, len);
}

int CChannel::GetCtrlMsg(ProcId dst, void* data, uint32_t& len) {
  UnlockQueueSetIt dst_it = unlock_queues_.find(dst);
  if ( unlock_queues_.end() == dst_it ) {
    return yarf::err::kChanDstNotExist;
  }

  len = 0;
  data = NULL;

  CUnlockQueue& queue = dst_it->second;
  return queue.GetCtrlMsg(data, len);
}


int CChannel::Recv(ProcId& src, char* data, uint32_t& len) {
  if ( unlock_queues_.end() ==  cur_recv_queue_ ) {
    cur_recv_queue_ = unlock_queues_.begin();
    cur_queue_recv_pkg_count_ = 0;;
  }

  if ( cur_queue_recv_pkg_count_ >= kMaxQueueRecvPkgCount ) {
    MoveToNextQueue();
  }


  const size_t unlock_queue_count = unlock_queues_.size();
  size_t loop_count = 0;
  int ret_code = yarf::err::kChanEmpty;
  while ( yarf::err::kChanEmpty == ret_code ) {
    CUnlockQueue& queue = cur_recv_queue_->second;
    ret_code = queue.Recv(data, len);
    src = queue.peer_id();
    if ( yarf::err::kChanEmpty == ret_code ) {
      MoveToNextQueue();
    } else if ( yarf::kOk == ret_code ) {
      cur_queue_recv_pkg_count_++;
      // cout << cur_queue_recv_pkg_count_ << endl;
      break;
    } else {
      // 发生错误，暂时不要影响其他通道
      MoveToNextQueue();
    }

    loop_count++;
    if ( loop_count >= unlock_queue_count ) {
      break;
    }
  }

  return ret_code;
}


int CChannel::Peek(ProcId& src, const char*& data, uint32_t& len) {
  if ( unlock_queues_.end() ==  cur_recv_queue_ ) {
    cur_recv_queue_ = unlock_queues_.begin();
    cur_queue_recv_pkg_count_ = 0;;
  }

  if ( cur_queue_recv_pkg_count_ >= kMaxQueueRecvPkgCount ) {
    MoveToNextQueue();
  }

  const size_t unlock_queue_count = unlock_queues_.size();
  size_t loop_count = 0;
  int ret_code = yarf::err::kChanEmpty;
  while ( yarf::err::kChanEmpty == ret_code ) {
    CUnlockQueue& queue = cur_recv_queue_->second;
    ret_code = queue.Peek(data, len);
    src = queue.peer_id();
    if ( yarf::err::kChanEmpty == ret_code ) {
      MoveToNextQueue();
    } else if ( yarf::kOk == ret_code ) {
      cur_queue_recv_pkg_count_++;
      break;
    } else {
      // 发生错误，暂时不要影响其他通道
      MoveToNextQueue();
    }

    loop_count++;
    if ( loop_count >= unlock_queue_count ) {
      break;
    }
  }

  return ret_code;
}


int CChannel::Drop(ProcId src) {
  UnlockQueueSetIt src_it = unlock_queues_.find(src);
  if ( unlock_queues_.end() == src_it ) {
    return yarf::err::kChanDstNotExist;
  }

  CUnlockQueue& queue = src_it->second;
  return queue.Drop();
}

