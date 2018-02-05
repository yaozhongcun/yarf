/* Copyright (c) 2016, Johnyao
 * All rights reserved.
 *
 * hash_pool.h
 *
 *  Created on: 2016年4月25日
 *      Author: YAOZHONGCUN
 */

#ifndef YARF_MEMORY_HASH_POOL_H_
#define YARF_MEMORY_HASH_POOL_H_


#include <string>
using std::string;
#include "yarf/memory/fixed_len_pool.h"
#include "yarf/memory/must_be_pod.h"

namespace yarf {
namespace memory {

template<typename KeyType, typename ValueType>
class HashPool {
  // hash表一个桶上的node
  typedef struct HashNode {
    KeyType key;
    ValueType value;
    size_t next;  // hash桶上指向下一个元素
  } HashNode;

  //  hash 表的表头
  typedef struct HashHeader {
    uint32_t bucket_num;
    uint32_t max_node;
    uint32_t node_num;
    size_t node_size;
    uint32_t extended;  // 供hash表的适用者适用
  } HashHeader;

 protected:
  HashHeader* header_;
  size_t* buckets_;  // bucket指向的是hash node在fixedlenpool中的偏移
  FixedLenPool mem_pool_;  // 使用fixedmempool来存储hash node
  string err_msg_;

 public:
  // 获取hash表需要使用的内存大小
  static size_t GetMaxSize(uint32_t bucket_num, uint32_t max_node) {
    return sizeof(HashHeader) + sizeof(buckets_[0]) * bucket_num
        + FixedLenPool::GetAllocSize(max_node, sizeof(HashNode));
  }

  // 获取hash的大小
  size_t GetMaxSize() {
    return GetMaxSize(header_->bucket_num, header_->max_node);
  }

 public:
  HashPool()
      : header_(NULL),
        buckets_(NULL) {
    static_assert(sizeof(HashHeader) % 8 == 0, "HashHeader size should be 8*N");
  }
  ~HashPool() {
  }

  // 访存函数
 public:
  inline uint32_t GetAllocNodeNum() const {
    return header_->node_num;
  }

  inline uint32_t Extended() const {
    return header_->extended;
  }

  inline void SetExtended(uint32_t value) {
    header_->extended = value;
  }

  inline size_t Ref(void* p) const {
    return mem_pool_.Ref(p);
  }

  inline void* Deref(size_t pos) const {
    return mem_pool_.Deref(pos);
  }

  inline const HashHeader* GetHeader() const {
    return header_;
  }

  inline const string& GetErrMsg() const {
    return err_msg_;
  }

  // 基本操作函数
 public:
  int32_t Init(void* mem, size_t mem_size, uint32_t bucket_num,
               uint32_t max_node, bool check_header) {
    if (mem_size < GetMaxSize(bucket_num, max_node)) {
      err_msg_ = "Init失败,非配的共享内存不足";
      return -1;
    }
    must_be_pod<KeyType>();
    must_be_pod<ValueType>();

    char* p = reinterpret_cast<char*>(mem);
    header_ = reinterpret_cast<HashHeader*>(p);
    if (check_header) {
      if (header_->bucket_num != bucket_num || header_->max_node != max_node
          || header_->node_num > max_node
          || header_->node_size != sizeof(HashNode)) {
        err_msg_ = "Init失败,check_header failed";
        return -1;
      }
    } else {
      header_->bucket_num = bucket_num;
      header_->max_node = max_node;
      header_->node_num = 0;
      header_->node_size = sizeof(HashNode);
      header_->extended = 0;
    }
    p += sizeof(HashHeader);

    buckets_ = reinterpret_cast<size_t*>(p);
    if (!check_header) {
      memset(buckets_, 0, sizeof(buckets_[0]) * header_->bucket_num);
    }
    p += sizeof(buckets_[0]) * header_->bucket_num;

    int ret = mem_pool_.Init(p, header_->max_node, sizeof(HashNode),
                             check_header);
    if (ret != 0) {
      err_msg_ = "Init失败,mem_pool_.Init失败: ";
      err_msg_.append(mem_pool_.GetErrorMsg());
      return -1;
    }

    return 0;
  }

  ValueType* Alloc(const KeyType& key) {
    HashNode* node = GetNode(key);
    if (node) {
      err_msg_ = "Alloc失败,error=key已经存在";
      return NULL;
    }

    node = reinterpret_cast<HashNode*>(mem_pool_.Alloc());
    if (!node) {
      err_msg_ = "Alloc失败,Alloc() return NULL,error: ";
      err_msg_.append(mem_pool_.GetErrorMsg());
      return NULL;
    }

    memcpy(&node->key, &key, sizeof(KeyType));
    int index = key % header_->bucket_num;
    if (buckets_[index] == 0) {
      node->next = 0;
      buckets_[index] = mem_pool_.Ref(node);
    } else {
      node->next = buckets_[index];
      buckets_[index] = mem_pool_.Ref(node);
    }

    header_->node_num++;

    return &node->value;
  }

  ValueType* Insert(const KeyType& key, const ValueType* value) {
    ValueType* alloc = Alloc(key);
    if (!alloc) {
      return NULL;
    }

    memcpy(alloc, value, sizeof(ValueType));

    return alloc;
  }

  ValueType* Get(const KeyType& key) {
    HashNode* node = GetNode(key);
    if (node) {
      return &node->value;
    }

    return NULL;
  }

  int32_t Free(const KeyType& key) {
    int32_t index = key % header_->bucket_num;
    HashNode* prev_node = NULL;
    HashNode* node = NULL;

    size_t p = buckets_[index];
    while (p) {
      node = reinterpret_cast<HashNode*>(mem_pool_.Deref(p));
      if (node->key == key) {
        if (prev_node) {
          prev_node->next = node->next;
        } else {
          buckets_[index] = node->next;
        }

        header_->node_num--;

        if (mem_pool_.Free(node) != 0) {
          err_msg_ = "Free失败,error: ";
          err_msg_.append(mem_pool_.GetErrorMsg());
          return -1;
        }
        return 0;
      }
      prev_node = node;
      p = node->next;
    }

    err_msg_ = "Free失败,key not found";
    return -1;
  }

 protected:
  HashNode* GetNode(const KeyType& key) {
    uint32_t index = key % header_->bucket_num;
    HashNode* node = NULL;

    size_t p = buckets_[index];
    while (p) {
      node = reinterpret_cast<HashNode*>(mem_pool_.Deref(p));
      if (node->key == key) {
        return node;
      }
      p = node->next;
    }

    return NULL;
  }
};

}  // namespace memory
}  // namespace yarf

#endif  // YARF_MEMORY_HASH_POOL_H_
