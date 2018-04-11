
#include "yarf/log.h" 
#include "yarf/thread/thread_pool.h"
#include "yarf/time/time.h"


#include "gtest/gtest.h"

using yarf::thread::CThreadPoolWorker;
using yarf::thread::CThreadPool;

class TestWorker : public CThreadPoolWorker {
 public:
  void Process(char* input, int32_t input_len, char* output, int32_t& output_len) {
  	memcpy(output, input, input_len);
    output_len = input_len;
  }
};

class TestPool : public CThreadPool {
 public:
  CThreadPoolWorker* CreateThread() {
      return (new TestWorker);
  }
};



TEST(thread, init ) {
  DEBUG_LOG("in test channel");

  TestPool main_pool;
  if (!main_pool.Init(10)) {
  	return;
  }

  
  char output[yarf::thread::kMaxOutboxLen];
  int busy_loop = 0;

  int test_num = 0;
  int sleep_loop = 0;

  uint64_t begin_millisec = g_time.Now().GetMillisec();
  do {
  	bool has_act_thread = false;
    do {
      int32_t output_len = yarf::thread::kMaxOutboxLen;
      if ( !main_pool.HandleBusyThread(output, output_len) ) {
        break;
      }
    
      ASSERT_EQ(output_len, static_cast<int32_t>(sizeof(test_num)));
      int* ret = reinterpret_cast<int*>(output);
      // DEBUG_LOG("get %d", *ret);
      has_act_thread = true;
    } while(true);	

    while (main_pool.HasFreeThread()) {
      has_act_thread = true;
      if (yarf::kOk != 
        main_pool.Process(
          reinterpret_cast<char*>(&test_num), sizeof(test_num))){
      	WARN_LOG("process error");
      }
      test_num++;
    }

    if (!has_act_thread) {
      busy_loop++;
      if (10 == busy_loop) {
      	usleep(2000);
      	busy_loop = 0;
      	sleep_loop++;
        // DEBUG_LOG("go sleep");
      }
    }

    if ( test_num > 100000) {
    	break;
    }

  } while(true);
  uint64_t end_millisec = g_time.Now().GetMillisec();

  DEBUG_LOG("cost %lu millisec, slepp loop %d", end_millisec - begin_millisec, sleep_loop);

}


