/* Copyright (c) 2017, Johnyao 
 * All rights reserved.
 * 
 * thread.h
 *
 *  Created on: 2016年11月22日
 *      Author: YAOZHONGCUN
 */

#ifndef YARF_THREAD_THREAD_H_
#define YARF_THREAD_THREAD_H_


#include <pthread.h>

namespace yarf {
namespace thread {

class CThread {
 public:
  CThread();
  virtual ~CThread() {}

  bool Start();
  void Stop();
  void Wait();

 protected:
  virtual void Run() = 0;

 private:
  static void* ThreadProc(void* param);

 protected:
  pthread_t thread_id_;
  bool stop_;
  bool start_;
};  // CThread


}  // namespace thread
}  // namespace yarf



#endif  // YARF_THREAD_THREAD_H_
