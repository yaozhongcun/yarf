/* Copyright (c) 2017, Johnyao 
 * All rights reserved.
 * 
 * thread.cc
 *
 *  Created on: 2016年11月22日
 *      Author: YAOZHONGCUN
 */


#include "yarf/thread/thread.h"

using yarf::thread::CThread;

CThread::CThread()
: stop_(false), start_(false) {
}

bool CThread::Start() {
  if ( start_ ) {
    return false;
  }

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  // pthread_attr_setstacksize(&attr, );
  // pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  if ( 0 != pthread_create(&thread_id_, &attr, ThreadProc,
                           reinterpret_cast<void*>(this)) ) {
    return false;
  }

  start_ = true;
  pthread_attr_destroy(&attr);
  return true;
}

void CThread::Wait() {
  // if start
  // if not detach stat
  pthread_join(thread_id_, NULL);
}

void CThread::Stop() {
  stop_ = true;
}

void* CThread::ThreadProc(void* param) {
  CThread* thread = reinterpret_cast<CThread*>(param);
  thread->Run();
  thread->start_ = false;
  return NULL;
}

