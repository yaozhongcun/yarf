/* Copyright (c) 2017, Johnyao
 * All rights reserved.
 *
 * channel_config.cc
 *
 *  Created on: 2016年5月5日
 *      Author: YAOZHONGCUN
 */


#include <time.h>

#include "./channel_config.h"
#include "yarf/err.h"
#include "./unlock_queue_impl.h"


using yarf::channel::GlobalConfigHead;
using yarf::channel::ChanConfig;
using yarf::channel::GlobalConfig;
using yarf::channel::UnlockQueueHead;
using yarf::memory::CShm;

bool GlobalConfig::IsValid() {
  // TODO:
  return true;
}

int GlobalConfig::GetChansByProcId(ProcId proc_id,
  vector<ChanConfig>& channel_config) {
  for (uint32_t chan_idx = 0; chan_idx < head_.channel_count; ++chan_idx) {
    const ChanConfig& tmp = channel_conf_[chan_idx];
    if (tmp.peer_id[0] == proc_id
      || tmp.peer_id[1] == proc_id
      ) {
      channel_config.push_back(tmp);
    }
  }

  return yarf::kOk;
}


GlobalConfig* GlobalConfig::Create(const GlobalConfigHead& head,
  const vector<ChanConfig>& channels) {
  GlobalConfig* global_config = CreateByKey(head.shm_key);

  for (size_t idx = 0; idx < channels.size(); ++idx) {
    const ChanConfig& channel_config = channels[idx];

    bool chan_exist = false;
    for (uint32_t chan_idx = 0;
      chan_idx < global_config->head_.channel_count; ++chan_idx) {
      if ((channel_config.peer_id[0]
          == global_config->channel_conf_[chan_idx].peer_id[0]
        && channel_config.peer_id[1]
          == global_config->channel_conf_[chan_idx].peer_id[1])
        || (channel_config.peer_id[0]
          == global_config->channel_conf_[chan_idx].peer_id[1]
        && channel_config.peer_id[1]
          == global_config->channel_conf_[chan_idx].peer_id[0])
        ) {
        chan_exist = true;
        break;
      }
    }
    if (chan_exist) {
      continue;
    }

    int ret_code = 0;
    if (0 !=
      (ret_code = global_config->AddChannel(
      channel_config.peer_id[0], channel_config.peer_id[1],
      channel_config.send_size, channel_config.recv_size))) {
      continue;
    }
  }
  return global_config;
}

GlobalConfig* GlobalConfig::CreateByKey(ShmKey key) {
  CShm shm;
  bool exist = false;
  int ret_code = shm.InitByKey(key, sizeof(GlobalConfig), exist, true);
  if (yarf::kOk != ret_code) {
    return NULL;
  }
  GlobalConfig* global_config = reinterpret_cast<GlobalConfig*>(shm.content());

  if (!exist) {
    global_config->magic_head_ = yarf::channel::kMagicNum;
    global_config->magic_tail_ = yarf::channel::kMagicNum;
    global_config->head_.channel_count = 0;
    global_config->head_.config_version = 0;
    time_t now = time(NULL);
    global_config->head_.create_time = static_cast<uint64_t>(now);
    global_config->head_.modify_time = static_cast<uint64_t>(now);
    global_config->head_.shm_key = key;
    global_config->head_.shm_size = sizeof(GlobalConfig);

    pthread_rwlockattr_t rwlockattr;
    pthread_rwlockattr_init(&rwlockattr);
    pthread_rwlockattr_setpshared(&rwlockattr, PTHREAD_PROCESS_SHARED);
    pthread_rwlock_init(&(global_config->head_.rw_lock),
      &rwlockattr);
  }
  return global_config;
}

int GlobalConfig::AddChannel(ProcId peer1, ProcId peer2,
  uint32_t send_size, uint32_t recv_size) {
  if (head_.channel_count >= kMaxChannelCount) {
    return yarf::err::kCfgMaxChan;
  }

  ChanConfig& channel_conf = channel_conf_[head_.channel_count];

  channel_conf.peer_id[0] = peer1;
  channel_conf.peer_id[1] = peer2;

  channel_conf.send_size = send_size;
  channel_conf.recv_size = recv_size;
  channel_conf.create_time = static_cast<uint64_t>(time(NULL));

  CShm send_shm;
  bool exist = false;
  int ret_code = 0;
  if (0 !=
    (ret_code = send_shm.InitByKey(
    IPC_PRIVATE, send_size + sizeof(UnlockQueueHead), exist, true))) {
    return ret_code;
  }
  channel_conf.send_shm_id = send_shm.shm_id();
  CShm recv_shm;
  if (0 !=
    (ret_code = recv_shm.InitByKey(
    IPC_PRIVATE, recv_size + sizeof(UnlockQueueHead), exist, true))) {
    send_shm.MarkDelete();
    return ret_code;
  }
  channel_conf.recv_shm_id = recv_shm.shm_id();

  // 初始化无锁队列的头部
  UnlockQueueHead* head
    = reinterpret_cast<UnlockQueueHead*>(send_shm.content());
  head->capacity = send_size;

  head = reinterpret_cast<UnlockQueueHead*>(recv_shm.content());
  head->capacity = recv_size;


  head_.channel_count++;

  return yarf::kOk;
}
