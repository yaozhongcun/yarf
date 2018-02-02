/* Copyright (c) 2017, Johnyao 
 * All rights reserved.
 *
 * channel_pack.h
 *
 *  Created on: 2016年5月5日
 *      Author: YAOZHONGCUN
 */

#ifndef YARF_CHANNEL_CHANNEL_PACK_H_
#define YARF_CHANNEL_CHANNEL_PACK_H_

#include "yarf/err.h"

namespace yarf {
namespace channel {

static const uint32_t kChannelAlignLevel = 8;

static const uint16_t kChannelCmdTransfer = 1;
static const uint16_t kChannelCmdPlaceHolder = 2;
static const uint16_t kChannelCmdHeart = 3;  // 暂未使用

struct ChannelPackHead {
  uint16_t version;  // 当前pack head版本 暂未启用
  uint16_t head_len;  // channel pack head 长度
  uint16_t body_len;  // channel pack body len
  uint16_t align_body_len;  // 字节对齐后的 channel pack body len
  uint16_t cmd;  // 命令字   数据传输  心跳包   伪造包
  uint16_t checksum;  // body 部分的校验码 暂未启用
  uint32_t seq;  // 序列号
  uint64_t time_stamp;  // time stamp 暂未启用
  uint32_t reserve[6];  // 保留扩展的字段

  explicit ChannelPackHead(uint16_t cur_cmd = kChannelCmdTransfer)
  : version(0),
    head_len(sizeof(ChannelPackHead)),
    body_len(0), align_body_len(0),
    cmd(cur_cmd),
    checksum(0), seq(0), time_stamp(0) {
  }

  static const ChannelPackHead place_hold_head;
};

}  // namespace channel
}  // namespace yarf

#endif  // YARF_CHANNEL_CHANNEL_PACK_H_
