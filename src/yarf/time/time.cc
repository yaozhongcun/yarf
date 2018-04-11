

#include "yarf/time/time.h"

#include <sys/time.h>

namespace yarf {
namespace time {

CTime::CTime() {
  Now();
}

CTime& CTime::Now() {
  gettimeofday(&tv_, NULL);
  // m_msec = UINT64(tv.tv_sec + m_offset) * 1000 + UINT64(tv.tv_usec / 1000);
  millisec_ = static_cast<uint64_t>(tv_.tv_sec) * 1000 
    + static_cast<uint64_t>(tv_.tv_usec) / 1000;
  return *this;
}

uint64_t CTime::GetMillisec() {
  return millisec_;
}

}  // namespace time
}  // namespace yarf

yarf::time::CTime g_time;