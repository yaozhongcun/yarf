/* Copyright (c) 2016, Johnyao
 * All rights reserved.
 *
 * fixed_len_pool.cc
 *
 *  Created on: 2016年4月22日
 *      Author: YAOZHONGCUN
 */
#include "yarf/memory/fixed_len_pool.h"
#include <stdio.h>

using yarf::memory::FixedLenPool;

int32_t FixedLenPool::Init(void* mem, uint32_t max_node, size_t node_size,
                           bool check_header) {
  if (!mem) {
    return -1;
  }

  header_ = reinterpret_cast<Header*>(mem);

  if (!check_header) {
    // 初始化内存头
    header_->max_size = GetAllocSize(max_node, node_size);
    header_->max_node = max_node;
    header_->alloc_size = sizeof(Header);
    header_->alloc_node = 0;
    header_->node_size = node_size;
    header_->block_size = GetBlockSize(node_size);
    header_->free_list = INVALID_REF;

    data_ = reinterpret_cast<char*>(header_) + sizeof(Header);
  } else {
    if (!CheckHeader(max_node, node_size, header_)) {
      snprintf(error_msg_, sizeof(error_msg_), "内存池头信息验证错误");
      return -1;
    }

    data_ = reinterpret_cast<char*>(header_) + sizeof(Header);
  }

  return 0;
}

void* FixedLenPool::Alloc(bool zero) {
  Block* p = GetFreeBlock();
  if (!p) {
    // 没有找到空闲块，需要从池中分配内存
    if (header_->alloc_size + header_->block_size <= header_->max_size) {
      p = reinterpret_cast<Block*>(Deref(header_->alloc_size));
      header_->alloc_size += header_->block_size;
      ++header_->alloc_node;
    } else {
      snprintf(error_msg_, sizeof(error_msg_), "Alloc错误,空间已满");
    }
  } else if (!IsValidBlock(p)) {
    snprintf(error_msg_, sizeof(error_msg_), "Alloc错误,空闲块非法%p,"
             "header=%p,max_size=%zu,alloc_size=%zu,block_size=%zu",
             p, header_, header_->max_size, header_->alloc_size,
             header_->block_size);
    p = NULL;
  }

  if (!p)
    return NULL;

  if (zero) {
    memset(p, 0, header_->block_size);
  }

  p->next = 0;

  return p->node;
}

int32_t FixedLenPool::Free(void* node) {
  Block* block = GetBlock(node);

  if (IsUsedBlock(block)) {  //  hash map使用自己节点内的next。  和这里没有冲突
    LinkFreeBlock(block);
  } else {
    snprintf(error_msg_, sizeof(error_msg_),
             "Free释放指针错误,指针%p为非法指针,"
             "header=%p,max_size=%zu,"
             "alloc_size=%zu,block_size=%zu",
             node, header_, header_->max_size,
             header_->alloc_size,
             header_->block_size);
    return -1;
  }

  return 0;
}

int32_t FixedLenPool::IsUsedNode(void* node) {
  Block* node_block = GetBlock(node);
  return IsUsedBlock(node_block);
}







