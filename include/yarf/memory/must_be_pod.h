/* Copyright (c) 2016, Johnyao
 * All rights reserved.
 *
 * must_be_pod.h
 *
 *  Created on: 2016年4月25日
 *      Author: YAOZHONGCUN
 */

#ifndef YARF_MEMORY_MUST_BE_POD_H_
#define YARF_MEMORY_MUST_BE_POD_H_

// 对于mem_pool里面存的数据类型，必须是POD类型，编译器做个检查

template<typename T> struct must_be_pod {
  union {
    T value;
  };
};



#endif  // YARF_MEMORY_MUST_BE_POD_H_
