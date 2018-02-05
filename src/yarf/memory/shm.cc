/* Copyright (c) 2016, Johnyao
 * All rights reserved.
 *
 * shm.cc
 *
 *  Created on: 2016年5月9日
 *      Author: YAOZHONGCUN
 */

#include "yarf/memory/shm.h"

#include <errno.h>

#include <cstring>

using yarf::memory::CShm;

int CShm::InitByKey(ShmKey key, size_t size, bool& exist, bool create) {
  exist = false;

  int shmid = shmget(key, size, 0644);  // 不创建
  if (shmid < 0) {
    if (ENOENT == errno) {  // 共享内存不存在
      if (!create) {
        return -1;
      } else {
        shmid = shmget(key, size, IPC_CREAT | IPC_EXCL | 0644);  // 创建
        if (shmid < 0) {
          if (errno != EEXIST) {
            return -1;
          }
          return -1;  // 该分支理论上不会执行到
        }
      }
    } else {
      return -1;
    }
  } else {
    exist = true;
  }

  void * p = shmat(shmid, NULL, 0);
  if (!p) {
    return -1;
  }
  if (!exist) {
    bzero(p, size);
  }

  content_ = p;
  size_ = size;
  exist_ = exist;
  shm_id_ = shmid;
  return 0;
}

int CShm::InitById(ShmId id, uint32_t size) {
  void * p = shmat(static_cast<int>(id), NULL, 0);
  if (!p) {
    return -1;
  }

  content_ = p;
  size_ = size;
  exist_ = true;
  shm_id_ = id;
  return 0;
}

int CShm::Detach() {
  if (shmdt(content_)) {
    return -1;
  }

  content_ = NULL;
  size_ = 0;
  exist_ = false;
  shm_id_ = 0;

  return 0;
}

int CShm::GetAttachNum(ShmId id, uint32_t& attach) {
  struct shmid_ds ds;
  int ret = shmctl(id, IPC_STAT, &ds);
  if (-1 == ret) {
    return -1;
  }
  attach = ds.shm_nattch;
  return 0;
}

int CShm::MarkDeleteByKey(ShmKey key) {
  int shmid = shmget(key, 0, 0644);
  if (shmid < 0) {
    return -1;
  }

  int ret = shmctl(shmid, IPC_RMID, NULL);
  if (-1 == ret) {
    return -1;
  }

  return 0;
}

int CShm::MarkDeleteById(ShmId id) {
  int ret = shmctl(id, IPC_RMID, NULL);
  if (-1 == ret) {
    return -1;
  }
  return 0;
}

int CShm::MarkDelete() {
  int ret = shmctl(shm_id_, IPC_RMID, NULL);
  if (-1 == ret) {
    return -1;
  }
  return 0;
}

