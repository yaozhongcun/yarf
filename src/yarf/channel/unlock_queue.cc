/* Copyright (c) 2017, Johnyao 
 * All rights reserved.
 *
 * unlock_queue.cc
 *
 *  Created on: 2016年5月3日
 *      Author: YAOZHONGCUN
 */


#include <cstring>
#include <iostream>
using std::cout;
using std::endl;

#include "./unlock_queue_impl.h"
#include "yarf/memory/misc.h"

using yarf::channel::CUnlockQueue;
using yarf::channel::ChannelPackHead;
using yarf::channel::CSendUnlockQueue;
using yarf::channel::CRecvUnlockQueue;

static const uint32_t kPackHeadSize = sizeof(ChannelPackHead);

int CUnlockQueue::Init(char* recv_mem, uint32_t recv_capacity,
  char* send_mem, uint32_t send_capacity,
  uint32_t peer_id) {
  peer_proc_id_ = peer_id;
  int ret_code = 0;
  if (0 != (ret_code = recver_.Init(recv_mem, recv_capacity))) {
    return ret_code;
  }
  if (0 != (ret_code = sender_.Init(send_mem, send_capacity))) {
    return ret_code;
  }
  return kOk;
}


int CSendUnlockQueue::Init(char* mem, uint32_t capacity) {
  if (NULL == mem) {
    return yarf::err::kArg;
  }
  if (capacity <=
    sizeof(UnlockQueueHead) + sizeof(ChannelPackHead)) {
    return yarf::err::kArg;
  }

  header_ = reinterpret_cast<UnlockQueueHead*>(mem);
  if (!header_->IsValid(capacity)) {
    return yarf::err::kChanHeadInvalid;
  }
  buff_ = mem + sizeof(UnlockQueueHead);

  return kOk;
}

int CSendUnlockQueue::Send(const char* data, uint32_t len) {
  const uint32_t pack_body_size = len;
  const uint32_t pack_size = kPackHeadSize + pack_body_size;
  const uint32_t align_pack_size = memory::GetAlignmentSize(pack_size);

  if (0 != (align_pack_size % kChannelAlignLevel)
    || align_pack_size < pack_size) {
    return yarf::err::kUnexpect;
  }

  const uint32_t free_size =
    (header_->capacity - header_->tail - 1 + header_->head) % header_->capacity;

  if (free_size < align_pack_size) {
    return yarf::err::kChanFull;
  }

  ChannelPackHead pack_header;
  pack_header.align_body_len = align_pack_size - kPackHeadSize;
  pack_header.body_len = pack_body_size;
  pack_header.seq = header_->seq;
  // TODO(johnyao): timestamp

  uint32_t cur_tail = header_->tail;
  uint32_t cur_head = header_->head;
  if (cur_tail >= cur_head) {
    // check
    const uint32_t tail_free_size = header_->capacity - cur_tail;

    if (tail_free_size < align_pack_size) {
      // tail_free_size == align_pack_size 注意要讲cur_tail置0
      if (cur_head <= align_pack_size) {
        // header_->head == align_pack_size会导致tail head 重叠
        // 队尾只放站位包头
        return yarf::err::kChanFull;
      }

      if (tail_free_size < kPackHeadSize) {
        // 剩余空间小于head大小不填充包头
        // cout << "send less than pack size" << tail_free_size
        // << ", pack head " << kPackHeadSize << endl;
      } else {
        // 填充占位包头
        // cout << "send place hold " << tail_free_size
        // << ", pack head " << kPackHeadSize << endl;
        memcpy(buff_ + cur_tail,
          &(ChannelPackHead::place_hold_head), kPackHeadSize);
      }
      cur_tail = 0;
    }
  }

  memcpy(buff_ + cur_tail, &(pack_header), kPackHeadSize);
  memcpy(buff_ + cur_tail + kPackHeadSize, data, pack_body_size);
  cur_tail = (cur_tail + align_pack_size) % header_->capacity;

  header_->tail = cur_tail;
  header_->seq++;

  return kOk;
}

int CRecvUnlockQueue::Init(char* mem, uint32_t capacity) {
  if (NULL == mem) {
    return yarf::err::kArg;
  }
  if (capacity <=
    sizeof(UnlockQueueHead) + sizeof(ChannelPackHead)) {
    return yarf::err::kArg;
  }

  header_ = reinterpret_cast<UnlockQueueHead*>(mem);
  if (!header_->IsValid(capacity)) {
    return yarf::err::kChanHeadInvalid;
  }
  buff_ = mem + sizeof(UnlockQueueHead);

  return kOk;
}


int CRecvUnlockQueue::GetNextPack(uint32_t &cur_head,
  uint32_t &align_pack_size,
  ChannelPackHead *&pack_head) {
  cur_head = header_->head;
  const uint32_t cur_tail = header_->tail;

  if (cur_tail == cur_head) {
    return yarf::err::kChanEmpty;
  }
  align_pack_size = 0;

  uint32_t alloc_size =
    (cur_tail + header_->capacity - cur_head) % header_->capacity;
  const uint32_t tail_alloc_size = header_->capacity - cur_head;
  if (tail_alloc_size < kPackHeadSize) {
    if (alloc_size < tail_alloc_size) {
      return yarf::err::kChanPackInvalid;
    }
    // cout << "encounter less tail " << cur_tail
    // << " head:" <<cur_head << endl;
    cur_head = 0;
  } else {
    ChannelPackHead* try_pack_head =
      reinterpret_cast<ChannelPackHead*>(buff_ + cur_head);
    if (kChannelCmdPlaceHolder == try_pack_head->cmd) {
      if (alloc_size < tail_alloc_size) {
        return yarf::err::kChanPackInvalid;
      }
      cur_head = 0;
      // cout << "encounter place hold: " << endl;
    }
  }

  alloc_size = (cur_tail + header_->capacity - cur_head) % header_->capacity;

  pack_head = reinterpret_cast<ChannelPackHead*>(buff_ + cur_head);
  align_pack_size = kPackHeadSize + pack_head->align_body_len;

  // 其他check
  if (kChannelCmdTransfer != pack_head->cmd) {
    return yarf::err::kUnexpect;
  }

  if (align_pack_size > alloc_size) {
    // cout << "pack size: " << align_pack_size << " alloc size: "
    // << alloc_size << endl << " cur_tail: " << cur_tail << " head: "
    // << header_->head << " cur_head: " << cur_head << endl;
    return yarf::err::kChanPackInvalid;
  }

  return kOk;
}

int CRecvUnlockQueue::Peek(const char*& data, uint32_t& len) {
  uint32_t cur_head = header_->head;
  ChannelPackHead* pack_head = NULL;
  uint32_t align_pack_size = 0;

  auto ret = GetNextPack(cur_head, align_pack_size, pack_head);
  if (ret < 0) {
    return ret;
  }
  if (NULL == pack_head) {
    return yarf::err::kUnexpect;
  }

  data = buff_ + cur_head + kPackHeadSize;
  len = pack_head->body_len;

  return kOk;
}

int CRecvUnlockQueue::Drop() {
  uint32_t cur_head = header_->head;
  ChannelPackHead* pack_head = NULL;
  uint32_t align_pack_size = 0;

  auto ret = GetNextPack(cur_head, align_pack_size, pack_head);
  if (ret < 0) {
    return ret;
  }
  if (NULL == pack_head) {
    return yarf::err::kUnexpect;
  }

  cur_head = (cur_head + align_pack_size) % header_->capacity;

  header_->head = cur_head;

  return kOk;
}

int CRecvUnlockQueue::Recv(char* data, uint32_t& len) {
  uint32_t cur_head = header_->head;
  ChannelPackHead* pack_head = NULL;
  uint32_t align_pack_size = 0;

  auto ret = GetNextPack(cur_head, align_pack_size, pack_head);
  if (ret < 0) {
    return ret;
  }
  if (NULL == pack_head) {
    return yarf::err::kUnexpect;
  }

  if (len < pack_head->body_len) {
    return yarf::err::kChanRecvSpaceLimit;
  }

  memcpy(data, buff_ + cur_head + kPackHeadSize, pack_head->body_len);
  len = pack_head->body_len;

  cur_head = (cur_head + align_pack_size) % header_->capacity;
  header_->head = cur_head;

  return kOk;
}

int yarf::channel::CreateUnlockQueue(
  char* mem, uint32_t capacity,
  IRecvUnlockQueue* &recv_queue, ISendUnlockQueue* &send_queue,
  bool reset ) {
  if (reset) {
    UnlockQueueHead* head = reinterpret_cast<UnlockQueueHead*>(mem);
    bzero(mem, sizeof(UnlockQueueHead));
    head->capacity = capacity - sizeof(UnlockQueueHead);
  }

  auto tmp_recv_queue = new CRecvUnlockQueue();
  auto tmp_send_queue = new CSendUnlockQueue();
  send_queue = tmp_send_queue;
  recv_queue = tmp_recv_queue;

  auto ret = tmp_recv_queue->Init(mem, capacity);
  if (ret < 0) {
    return ret;
  }

  ret = tmp_send_queue->Init(mem, capacity);

  return ret;
}
