/*
 * thread_pool.h
 *
 *  Created on: 2016年12月1日
 *      Author: YAOZHONGCUN
 */

#ifndef YARF_THREAD_THREAD_POOL_H_
#define YARF_THREAD_THREAD_POOL_H_


#include "thread.h"

#include "yarf/err.h"
#include "yarf/log.h"

#include <vector>
using std::vector;
#include <list>
using std::list;


namespace yarf {
namespace thread {

static const int kMaxInboxLen = 8*1024;
static const int kMaxOutboxLen = 16*1024;

class CThreadPoolWorker : public CThread {
 public:
     CThreadPoolWorker();

  bool Init();
  
  bool IsBusy();

  int DeliverWork(const char* input, int32_t input_len);  // for main thread

  int CollectRsp(char* output, int32_t& output_len);  // for main thread

 protected:
  void Run();

  virtual void Process(char* input, int32_t input_len, char* output, int32_t& output_len) = 0;

 protected:
  volatile bool busy_;
  pthread_mutex_t wake_signal_;
  volatile int inbox_len_;  // main thread: if inbox_len_ == 0,  write make it > 0
  char inbox_[kMaxInboxLen];
  volatile int outbox_len_;  // main thread: outbox_len_ if this > 0 read, make it == 0
  char outbox_[kMaxOutboxLen];
};


// 
// 线程池
// 
// 主线程中按如下方式使用
// while (HandleBusyThread(...)) {
//   // send to rpc
// }
//
// while (HasFreeThread()) {
//   // get from rpc 
//   Process(...);
// }
//
// 主线程中需要控制all busy, all free状态, 
// all busy需要计时,  适当处理堆积的包
// all free可以进入睡眠时间长一些

class CThreadPool {

 public:
  CThreadPool() {}
  virtual ~CThreadPool();
 public:
  bool Init(int32_t pool_size);

  // TODO(johnyao): refactor with rpc
  // ret -1, error; 0 no process; 1 done
  bool HandleBusyThread(char* output, int32_t& output_len);

  bool HasFreeThread();

  int Process(const char* input, int32_t input_len);

 protected:
  virtual CThreadPoolWorker* CreateThread() = 0;

 private:
  list<CThreadPoolWorker*> free_threads_;
  list<CThreadPoolWorker*> busy_threads_;
  //

};

}  // namespace thread
}  // namespace yarf


#endif  // YARF_THREAD_THREAD_POOL_H_
