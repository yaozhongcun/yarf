/*
 * time.h
 *
 *  Created on: 2018年4月10日
 *      Author: YAOZHONGCUN
 */

#pragma once

#include "yarf/yarf.h"
#include <time.h>

namespace yarf {
namespace time {

class CTime {
 public:
  CTime();

  CTime& Now();

  uint64_t GetMillisec();

  private:
    struct timeval	tv_;
    uint64_t millisec_;
};

}  // namespace time
}  // namespace yarf


extern yarf::time::CTime g_time;

