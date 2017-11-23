

#include "gtest/gtest.h"

#include "yarf/log.h" 
#include "yarf/channel/unlock_queue.h"
#include "yarf/thread/thread.h"

using namespace yarf;
using namespace yarf::channel;
using namespace yarf::thread;

static const uint32_t kMaxLoop = 1000;
class CRecvThread : public CThread {
 public:
  void Run() {
    for (uint32_t x = 0; x < kMaxLoop; x++) {
      uint32_t tar = 0;
      uint32_t len = sizeof(tar);
      int ret = 0;
      do {
        ret = recv_queue_->Recv(reinterpret_cast<char*>(&tar), len);
      } while ( err::kChanEmpty == ret );
      ASSERT_EQ(ret, 0);
      ASSERT_EQ(len, sizeof(tar));
      ASSERT_EQ(tar, x);
      // DEBUG_LOG("recv %u", tar);
    }
  }
public:
  IRecvUnlockQueue* recv_queue_;
};

TEST(unlockqueue, init) {
  DEBUG_LOG("in test unlock");
  uint32_t capacity = 2*1024;
  char* mem = new char[capacity];
  IRecvUnlockQueue* recv_queue = NULL;
  ISendUnlockQueue* send_queue = NULL;
  ASSERT_NE( CreateUnlockQueue(mem, 10, recv_queue, send_queue), 0);
  ASSERT_EQ( CreateUnlockQueue(mem, capacity, recv_queue, send_queue), 0);
  ASSERT_NE( recv_queue, nullptr );
  ASSERT_NE( send_queue, nullptr );

  CRecvThread recv_thread;
  recv_thread.recv_queue_ = recv_queue;
  recv_thread.Start();

  uint32_t x = 0;
  for (; x < kMaxLoop; x++) {
    auto ret = 0;
    do {
      // DEBUG_LOG("send %u", x);
      ret = send_queue->Send(reinterpret_cast<char*>(&x), sizeof(x));
    } while (ret == err::kChanFull);
    ASSERT_EQ(ret, 0);
  }

  recv_thread.Wait();
}

