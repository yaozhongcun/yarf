/* Copyright (c) 2016, Johnyao
 * All rights reserved.
 *
 * shm.h
 *
 *  Created on: 2016年5月9日
 *      Author: YAOZHONGCUN
 */

#ifndef YARF_MEMORY_SHM_H_
#define YARF_MEMORY_SHM_H_

#ifndef WIN32

#include <sys/ipc.h>
#include <sys/shm.h>

typedef int ShmId;
typedef key_t ShmKey;

#else

#endif

#include <stdint.h>
#include <stddef.h>

namespace yarf {
namespace memory {

class CShm {
 public:
  CShm()
      : content_(NULL),
        size_(0),
        exist_(false),
        shm_id_(0) {
  }

  int InitByKey(ShmKey key, size_t size, bool& exist, bool create = false);

  int InitById(ShmId id, uint32_t size);

  int Detach();

  static int GetAttachNum(ShmId id, uint32_t& attach);

  static int MarkDeleteByKey(ShmKey key);

  static int MarkDeleteById(ShmId id);

  int MarkDelete();

  void* content() {
    return content_;
  }

  uint32_t size() {
    return size_;
  }

  bool exist() {
    return !exist_;
  }

  ShmId shm_id() {
    return shm_id_;
  }

 private:
  void* content_;
  uint32_t size_;
  bool exist_;
  ShmId shm_id_;
};

}  // namespace memory
}  // namespace yarf

#endif  // YARF_MEMORY_SHM_H_
