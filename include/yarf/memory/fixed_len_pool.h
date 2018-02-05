/* Copyright (c) 2016, Johnyao
 * All rights reserved.
 *
 * fixed_len_pool.h
 *
 *  Created on: 2016年4月22日
 *      Author: YAOZHONGCUN
 */

#ifndef YARF_MEMORY_FIXED_LEN_POOL_H_
#define YARF_MEMORY_FIXED_LEN_POOL_H_

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <type_traits>

namespace yarf {
namespace memory {

class FixedLenPool {
 public:
  static const size_t INVALID_REF = static_cast<size_t>(-1);

  // 每个node占用的block大小
  typedef struct Block {
    // next = 0, 表示已经分配
    // next = INVALID_REF, 表示空闲队列结尾
    // next = else. 链接空闲队列
    size_t next;
    char node[0];
  } Block;

  static const size_t NODE_OFFSET = offsetof(Block, node);

  // 管理内存池的header信息
  typedef struct Header {
    size_t max_size;  // 最大可分配大小
    uint32_t max_node;  // 最大节点数
    size_t alloc_size;  // 已分配大小
    uint32_t alloc_node;  // 已分配节点数
    size_t node_size;  // 节点实际大小
    size_t block_size;  // 分配块大小
    size_t free_list;  // 空闲链
  } Header;

 protected:
  Header* header_;
  void* data_;
  char error_msg_[256];

 public:
  //    计算分配max_node个节点至少需要多少内存
  //    @param max_node 最多可能同时分配多少个节点
  //    @param node_size 每个节点多大
  //    @return 需要的字节数
  static size_t GetAllocSize(uint32_t max_node, size_t node_size) {
    return sizeof(Header) + max_node * GetBlockSize(node_size);
  }

  // 计算一个node占用的块大小
  // @param node_size 节点大小
  // @return node占用的字节大小
  static size_t GetBlockSize(size_t node_size) {
    // 分配块大小按照8字节对齐
    return ((node_size + NODE_OFFSET + 7) & ~7);
  }

 protected:
  size_t GetBlockSize() {
    return GetBlockSize(header_->node_size);
  }

 public:
  FixedLenPool()
      : header_(NULL),
        data_(NULL) {
    memset(error_msg_, 0, sizeof(error_msg_));
    static_assert(sizeof(Header) % 8 == 0, "Header size should be 8*N");
  }

  virtual ~FixedLenPool() {
  }

 public:
  // 根据指针计算相对表头的位置
  inline size_t Ref(void* p) const {
    return (size_t) ((intptr_t) p - (intptr_t) (header_));
  }

  //  根据相对位置计算指针
  inline void* Deref(size_t pos) const {
    return reinterpret_cast<char*>( header_ + pos );
  }

  inline const Header* GetHeader() const {
    return header_;
  }

  inline const char* GetErrorMsg() const {
    return error_msg_;
  }

  // 基本使用函数
 public:
  // 初始化 内存池
  int32_t Init(void* mem, uint32_t max_node, size_t node_size,
               bool check_header = false);

  //  分配节点
  void* Alloc(bool zero = true);

  // 删除节点
  int32_t Free(void* node);

  // 节点是否合法
  int32_t IsUsedNode(void* node);
  // 遍历相关函数
 public:
  // 根据节点指针获取block指针
  Block* GetBlock(void* node) {
    return reinterpret_cast<Block*>(
        reinterpret_cast<char*>(node) - NODE_OFFSET);
  }

  // 获取第一个block指针
  const Block* GetFirstBlock() const {
    const Block* block = reinterpret_cast<Block*>(data_);
    return (IsValidBlock(block) ? block : NULL);
  }

  // 获取下一个block指针
  const Block* GetNextBlock(const Block* block) const {
    if (!block)
      return GetFirstBlock();

    const Block* next = reinterpret_cast<const Block*>(
        reinterpret_cast<const char*>(block) + header_->block_size);
    return (IsValidBlock(next) ? next : NULL);
  }

  // 获取下一个已经使用的block指针
  // 用作hash map的内存池的时候不适合
  const Block* GetNextUsedBlock(const Block* block = NULL) const {
    while ((block = GetNextBlock(block))) {
      if (block->next == 0) {
        return block;
      }
    }

    return NULL;
  }

 protected:
  // 检查表头是否合法
  static bool CheckHeader(uint32_t max_node, size_t node_size, Header* header) {
    size_t max_size = GetAllocSize(max_node, node_size);
    // 验证内存头部信息是否正确
    if ((header->max_size != max_size) || (header->max_node != max_node)
        || (header->alloc_size > max_size)
        || (sizeof(Header) + header->alloc_node * header->block_size
            != header->alloc_size)
        || (header->node_size != node_size)
        || (header->block_size != GetBlockSize(node_size)))  {
      // TODO(john): verify free list?
      return false;
    }

    return true;
  }

  // 是否是合法的block
  bool IsValidBlock(const Block* block) const {
    return !(block < data_
        || (size_t) block + header_->block_size
            > (size_t) header_ + header_->alloc_size
        || Ref(const_cast<Block*>(block)) % header_->block_size
            != sizeof(Header) % header_->block_size);
  }

  // 对于hash map不适用
  bool IsUsedBlock(const Block* block) const {
    return (IsValidBlock(block) && block->next == 0);
  }

  // 连接空闲块
  void LinkFreeBlock(Block* p) {
    p->next = header_->free_list;
    header_->free_list = Ref(p);
  }

  // 获取空闲 block
  Block* GetFreeBlock() {
    Block* p = NULL;
    if (header_->free_list == INVALID_REF) {
      p = NULL;
    } else {
      p = reinterpret_cast<Block*>(Deref(header_->free_list));
      header_->free_list = p->next;
    }
    return p;
  }
};

}  // namespace memory
}  // namespace yarf



#endif  // YARF_MEMORY_FIXED_LEN_POOL_H_
