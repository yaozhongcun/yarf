/*
 * thread_pool.cc
 *
 *  Created on: 2016年12月2日
 *      Author: YAOZHONGCUN
 */

#include "yarf/thread/thread_pool.h"
#include "yarf/log.h"
#include "yarf/err.h"

namespace yarf {
namespace thread {

CThreadPoolWorker::CThreadPoolWorker() {
}

bool CThreadPoolWorker::Init() {

  pthread_mutexattr_t atrr;
  auto ret = pthread_mutexattr_init(&atrr);
  if (ret != 0) { return false; }
  ret = pthread_mutex_init(&wake_signal_, &atrr);
  if (ret != 0) { return false; }
  ret = pthread_mutex_lock(&wake_signal_);  // 占有锁,  等待消息到来解锁
  if (ret != 0) { return false; }
  
  busy_ = false;
  inbox_len_ = 0;
  outbox_len_ = 0;

  return true;
}

bool CThreadPoolWorker::IsBusy() {
  return busy_;
}

int CThreadPoolWorker::DeliverWork(const char* input, int32_t input_len) {
  if (inbox_len_ != 0) {
    return yarf::err::kUnexpect;
  }

  if (input_len > kMaxInboxLen) {
    return yarf::err::kSpaceLimit;
  }

  memcpy(inbox_, input, input_len);

  inbox_len_ = input_len;

  auto ret = pthread_mutex_unlock(&wake_signal_);  // 消息到,解锁唤醒,此时inbox len非0
  if (ret != 0) { WARN_LOG("unlock ret err"); }

  return yarf::kOk;
}

int CThreadPoolWorker::CollectRsp(char* output, int32_t& output_len) {
  if (output_len < outbox_len_) {
    return yarf::err::kSpaceLimit;
  }
  if (0 == outbox_len_) {
    return yarf::err::kUnexpect;
  }
  memcpy(output, outbox_, outbox_len_);
  output_len = outbox_len_;
  outbox_len_ = 0;
  return yarf::kOk;
}

void CThreadPoolWorker::Run() {
  while (!stop_) {
    while (inbox_len_ == 0) {
      int ret = pthread_mutex_lock(&wake_signal_);
      if (ret != 0) { WARN_LOG("lock wake signal"); }
    }

    busy_ = true;
    while (outbox_len_ != 0) {
      WARN_LOG("should not happen here");
      usleep(1000);
    }
    int cur_outbox_len = kMaxOutboxLen;
    Process(inbox_, inbox_len_, outbox_, cur_outbox_len);
    outbox_len_ = cur_outbox_len;
    inbox_len_ = 0;
    busy_ = false;
  }
}

/////////////////////////////////////////////////////
// thread pool 实现
CThreadPool::~CThreadPool() {

}

bool CThreadPool::Init(int32_t pool_size) {
  
  int32_t cur_size = static_cast<int32_t>(free_threads_.size() + busy_threads_.size());
  if (cur_size >= pool_size) {
    return true;
  }
  cur_size = pool_size - cur_size;
  if (cur_size <= 0) {
    return true;
  }

  for (int32_t idx = 0; idx < cur_size; ++idx) {
    auto worker = CreateThread();
    if (!worker->Init()) {
      return false;
    }
    worker->Start();
    free_threads_.push_back(worker);
  }

  return true;
}

bool CThreadPool::HasFreeThread() {
  return ( 0 != free_threads_.size() );
}

int CThreadPool::Process(const char* input, int32_t input_len) {
  if (0 == free_threads_.size()) {
    return yarf::err::kUnexpect;
  }
  auto worker = free_threads_.front();
  if (nullptr == worker) {
    ERR_LOG("free worker is null, delete it");
    free_threads_.pop_front();
    return yarf::err::kUnexpect;
  }
  if (yarf::kOk != worker->DeliverWork(input, input_len)) {
      ERR_LOG("deliver work err");
      return yarf::err::kUnexpect;
  }
  free_threads_.pop_front();
  busy_threads_.push_back(worker);
  return yarf::kOk;
}

bool CThreadPool::HandleBusyThread(char* output, int32_t& output_len) {
  if (busy_threads_.size() == 0) {
    return false;
  }
  for (auto it = busy_threads_.begin(); it != busy_threads_.end(); ) {
    if (nullptr == (*it)) {
      ERR_LOG("busy worker is null, delete it");
      it = busy_threads_.erase(it);
      continue;
    }
    auto worker = (*it);
    if (!worker->IsBusy()) {
      if (yarf::kOk == worker->CollectRsp(output, output_len)) {
        busy_threads_.erase(it);
        free_threads_.push_back(worker);
        return true;
      }
    }
    ++it;
  }
  return false;
}


}  // namespace thread
}  // namespace yarf

