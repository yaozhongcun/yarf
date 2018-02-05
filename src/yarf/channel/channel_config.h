/* Copyright (c) 2017, Johnyao 
 * All rights reserved.
 *
 * channel_config.h
 *
 *  Created on: 2016年5月5日
 *      Author: YAOZHONGCUN
 */

#ifndef YARF_CHANNEL_CHANNEL_CONFIG_H_
#define YARF_CHANNEL_CHANNEL_CONFIG_H_

#include <pthread.h>

#include <vector>
using std::vector;

#include "yarf/channel/channel.h"
#include "yarf/memory/shm.h"

namespace yarf {
namespace channel {

static const uint32_t kMaxChannelCount = 2000;
static const uint16_t kMagicNum = 0x0809;

static const uint32_t kGlobalConfigHeadReserve = 8;
static const uint32_t kChanReserve = 8;

//
// 全局的channel配置（包含某台机器上的所有channel配置）头信息
//
struct GlobalConfigHead {
  uint32_t shm_key;   // 共享内存配置的key
  uint32_t shm_size;  // 共享内存配置的大小
  time_t create_time;  // 创建该配置的时间
  time_t modify_time;  // 最后一次修改的时间
  uint32_t channel_count;  // channel的数量
  pthread_rwlock_t rw_lock;  // 读写该共享内存的而配置
  uint32_t config_version;  // 该内存结构的版本  初始版本0
  uint32_t reserve[kGlobalConfigHeadReserve];
};

// 某个channel中的具体配置
//    存储在共享内存中
struct ChanConfig {
  ProcId peer_id[2];  // channel的通信双方的地址

  ShmId send_shm_id;  // send shm id
  ShmId recv_shm_id;  // recv shm id

  uint32_t send_size;  // peer 0 -> 1的队列大小. 不包含无锁队列的头部
  uint32_t recv_size;  // peer 1 -> 0 的队列大小. 不包含无锁队列的头部

  time_t create_time;  // channel 创建日期

  uint32_t reserve[kChanReserve];
};

// 全局的channel配置（包含某台机器上的所有channel配置）
//   使用共享内存存储
//   共享内存  整体大小不能随版本来变化，字段位置随版本递增，但不能调整。
struct GlobalConfig {
 private:
  uint16_t magic_head_;   // magic num 用于校验
  GlobalConfigHead head_;  // 全局配置头部
  ChanConfig channel_conf_[kMaxChannelCount];  // channel配置
  uint16_t magic_tail_;  // magic num 用于校验

 public:
  // 检验全局配置共享内存是否合法
  bool IsValid();
  // 获取proc id对应进程的所有通道配置
  int GetChansByProcId(ProcId proc_id, vector<ChanConfig>& channel_config);

  // 给工具使用，使用配置文件来创建共享内存配置
  static GlobalConfig* Create(const GlobalConfigHead& head,
    const vector<ChanConfig>& channels);

  // 输出所有信息
  void Output() {}

 private:
  // 供CreateByConf使用。 使用key来创建贡献内存
  static GlobalConfig* CreateByKey(ShmKey key);
  // 供CreateByConf使用。增加一个新的进程间通道配置。
  int AddChannel(ProcId peer1, ProcId peer2,
    uint32_t send_size, uint32_t recv_size);
};

}  // namespace channel
}  // namespace yarf

#endif  // YARF_CHANNEL_CHANNEL_CONFIG_H_
