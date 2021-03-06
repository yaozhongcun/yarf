/* Copyright (c) 2017, Johnyao
 * All rights reserved.
 *
 * misc.cc
 *
 *  Created on: 2017��11��22��
 *      Author: YAOZHONGCUN
 */

#include "yarf/err.h"
#include "yarf/memory/misc.h"



uint32_t yarf::memory::GetAlignmentSize(uint32_t size,
  uint32_t alignment_level) {
  if (8 == alignment_level) {
    return ((size + 7) & ~7);
  } else if (4 == alignment_level) {
    return ((size + 3) & ~3);
  } else if (2 == alignment_level) {
    return  ((size + 1) & ~1);
  }

  return size;
}

